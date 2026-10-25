const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = process.env.PORT || 3000;
const ROOT_DIR = path.resolve(__dirname, '..');
const COMPILER_BIN = path.join(ROOT_DIR, 'compiler');
const SCRIPTS_DIR = path.join(ROOT_DIR, 'test_scripts');
const PUBLIC_DIR = path.join(__dirname, 'public');

// MIME types for static assets
const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.svg': 'image/svg+xml',
    '.png': 'image/png'
};

function parseCompilerOutput(stdout, stderr, exitCode) {
    const raw = (stdout || '') + (stderr ? ('\n' + stderr) : '');
    const lines = raw.split('\n');

    let verificationPass = raw.includes('Static verification succeeded.');
    let verificationFailed = raw.includes('Static verification failed.') || raw.includes('Verification FAIL:');
    
    const checks = [];
    const counterexamples = [];
    let inCounterexamples = false;

    for (let i = 0; i < lines.length; i++) {
        const line = lines[i].trim();
        if (line.startsWith('Verification PASS:')) {
            checks.push({
                status: 'PASS',
                title: line.replace('Verification PASS:', '').trim()
            });
            inCounterexamples = false;
        } else if (line.startsWith('Verification FAIL:')) {
            checks.push({
                status: 'FAIL',
                title: line.replace('Verification FAIL:', '').trim()
            });
            inCounterexamples = false;
        } else if (line.startsWith('Counterexample values:')) {
            inCounterexamples = true;
        } else if (inCounterexamples) {
            const match = line.match(/^([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.*)$/);
            if (match) {
                counterexamples.push({
                    name: match[1],
                    value: match[2].trim()
                });
            } else if (line.startsWith('Verification') || line.startsWith('Static') || line.startsWith('AST:')) {
                inCounterexamples = false;
            }
        }
    }

    let ast = '';
    const astStart = raw.indexOf('AST:');
    const outputStart = raw.indexOf('OUTPUT:');
    if (astStart !== -1) {
        if (outputStart !== -1 && outputStart > astStart) {
            ast = raw.substring(astStart + 4, outputStart).trim();
        } else {
            ast = raw.substring(astStart + 4).trim();
        }
    }

    let output = '';
    if (outputStart !== -1) {
        output = raw.substring(outputStart + 7).trim();
    }

    return {
        exitCode,
        success: exitCode === 0,
        verificationPass,
        verificationFailed,
        checks,
        counterexamples,
        ast,
        output,
        raw
    };
}

function sendJson(res, statusCode, data) {
    res.writeHead(statusCode, {
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*',
        'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
        'Access-Control-Allow-Headers': 'Content-Type'
    });
    res.end(JSON.stringify(data));
}

function serveStatic(req, res, filePath) {
    fs.readFile(filePath, (err, content) => {
        if (err) {
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('404 Not Found');
            return;
        }
        const ext = path.extname(filePath).toLowerCase();
        const mime = MIME_TYPES[ext] || 'application/octet-stream';
        res.writeHead(200, { 'Content-Type': mime });
        res.end(content);
    });
}

const server = http.createServer((req, res) => {
    // Handle CORS preflight
    if (req.method === 'OPTIONS') {
        res.writeHead(204, {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Access-Control-Allow-Headers': 'Content-Type'
        });
        res.end();
        return;
    }

    const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    const pathname = url.pathname;

    // API: GET /api/examples
    if (pathname === '/api/examples' && req.method === 'GET') {
        try {
            if (!fs.existsSync(SCRIPTS_DIR)) {
                return sendJson(res, 200, { examples: [] });
            }

            const files = fs.readdirSync(SCRIPTS_DIR)
                .filter(f => f.endsWith('.ver'))
                .sort();

            const examples = files.map(file => {
                const filePath = path.join(SCRIPTS_DIR, file);
                const content = fs.readFileSync(filePath, 'utf-8');
                const cleanName = file.replace(/^\d+_/, '').replace(/\.ver$/, '');
                const title = cleanName.split('_').map(w => w.charAt(0).toUpperCase() + w.slice(1)).join(' ');

                return {
                    id: file,
                    title: `${file.slice(0, 2)} - ${title}`,
                    filename: file,
                    content
                };
            });

            return sendJson(res, 200, { examples });
        } catch (err) {
            return sendJson(res, 500, { error: err.message });
        }
    }

    // API: POST /api/save - Save file to test_scripts directory
    if (pathname === '/api/save' && req.method === 'POST') {
        let body = '';
        req.on('data', chunk => { body += chunk; });
        req.on('end', () => {
            try {
                const parsedBody = JSON.parse(body || '{}');
                let { filename, content } = parsedBody;

                if (!filename || typeof filename !== 'string') {
                    return sendJson(res, 400, { error: 'Filename is required' });
                }
                if (typeof content !== 'string') {
                    content = '';
                }

                // Sanitize filename and ensure .ver extension
                filename = path.basename(filename.trim());
                if (!filename.endsWith('.ver')) {
                    filename += '.ver';
                }

                const targetPath = path.join(SCRIPTS_DIR, filename);
                fs.writeFileSync(targetPath, content, 'utf-8');

                return sendJson(res, 200, { success: true, filename });
            } catch (err) {
                return sendJson(res, 500, { error: err.message });
            }
        });
        return;
    }

    // API: POST /api/execute
    if (pathname === '/api/execute' && req.method === 'POST') {
        let body = '';
        req.on('data', chunk => { body += chunk; });
        req.on('end', () => {
            try {
                const parsedBody = JSON.parse(body || '{}');
                const code = parsedBody.code;

                if (typeof code !== 'string') {
                    return sendJson(res, 400, { error: 'Code string is required' });
                }

                const tempFile = path.join(ROOT_DIR, `.tmp_${Date.now()}_run.ver`);
                fs.writeFileSync(tempFile, code, 'utf-8');

                const env = { 
                    ...process.env, 
                    PATH: `/opt/homebrew/bin:/usr/local/bin:${process.env.PATH || ''}` 
                };

                const proc = spawn(COMPILER_BIN, [tempFile], { 
                    cwd: ROOT_DIR,
                    env
                });

                let stdout = '';
                let stderr = '';

                proc.stdout.on('data', data => { stdout += data.toString(); });
                proc.stderr.on('data', data => { stderr += data.toString(); });

                proc.on('close', code => {
                    if (fs.existsSync(tempFile)) {
                        try { fs.unlinkSync(tempFile); } catch (e) {}
                    }
                    const parsed = parseCompilerOutput(stdout, stderr, code);
                    sendJson(res, 200, parsed);
                });

                proc.on('error', err => {
                    if (fs.existsSync(tempFile)) {
                        try { fs.unlinkSync(tempFile); } catch (e) {}
                    }
                    sendJson(res, 500, { error: `Failed to spawn compiler: ${err.message}` });
                });
            } catch (err) {
                sendJson(res, 500, { error: err.message });
            }
        });
        return;
    }

    // Serve static files from public/
    let reqPath = pathname === '/' ? '/index.html' : pathname;
    const safePath = path.normalize(reqPath).replace(/^(\.\.[\/\\])+/, '');
    const fullPath = path.join(PUBLIC_DIR, safePath);

    serveStatic(req, res, fullPath);
});

server.listen(PORT, () => {
    console.log(`Vera IDE running at http://localhost:${PORT}`);
});
