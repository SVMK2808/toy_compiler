document.addEventListener('DOMContentLoaded', () => {
    // DOM Elements
    const codeEditor = document.getElementById('code-editor');
    const highlightLayer = document.getElementById('highlight-layer');
    const lineNumbers = document.getElementById('line-numbers');
    const cursorPosition = document.getElementById('cursor-position');
    const currentFilename = document.getElementById('current-filename');
    const examplesContainer = document.getElementById('examples-container');
    const examplesCount = document.getElementById('examples-count');

    const btnRun = document.getElementById('btn-run');
    const btnVerify = document.getElementById('btn-verify');
    const btnClear = document.getElementById('btn-clear');
    const btnNew = document.getElementById('btn-new');
    const btnSave = document.getElementById('btn-save');
    const btnSidebarNew = document.getElementById('btn-sidebar-new');
    const fileDirtyIndicator = document.getElementById('file-dirty-indicator');
    const systemStatus = document.getElementById('system-status');

    const statusCard = document.getElementById('status-card');
    const statusIcon = document.getElementById('status-icon');
    const statusHeading = document.getElementById('status-heading');
    const statusDescription = document.getElementById('status-description');

    const counterexamplesSection = document.getElementById('counterexamples-section');
    const counterexamplesContainer = document.getElementById('counterexamples-container');
    const checksContainer = document.getElementById('checks-container');

    const terminalOutput = document.getElementById('terminal-output');
    const astOutput = document.getElementById('ast-output');
    const logsOutput = document.getElementById('logs-output');
    const dockTabs = document.querySelectorAll('.dock-tab');
    const tabPanes = document.querySelectorAll('.tab-pane');

    let currentActiveId = null;

    // Syntax Highlighting Tokens
    const KEYWORDS = ['let', 'fn', 'return', 'if', 'else', 'while', 'for', 'do', 'print'];
    const VERIFY_WORDS = ['requires', 'ensures', 'invariant', 'assert'];
    const BOOLEANS = ['true', 'false'];

    function escapeHtml(text) {
        return text
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
    }

    function highlightSyntax(code) {
        const lines = code.split('\n');
        const highlightedLines = lines.map(line => {
            // Check for single line comments
            const commentIndex = line.indexOf('//');
            let codePart = commentIndex !== -1 ? line.substring(0, commentIndex) : line;
            let commentPart = commentIndex !== -1 ? line.substring(commentIndex) : '';

            // Tokenize codePart
            let result = escapeHtml(codePart);

            // Highlight keywords
            KEYWORDS.forEach(kw => {
                const regex = new RegExp(`\\b(${kw})\\b`, 'g');
                result = result.replace(regex, '<span class="hl-keyword">$1</span>');
            });

            // Highlight verification constructs
            VERIFY_WORDS.forEach(vw => {
                const regex = new RegExp(`\\b(${vw})\\b`, 'g');
                result = result.replace(regex, '<span class="hl-verify">$1</span>');
            });

            // Highlight booleans
            BOOLEANS.forEach(b => {
                const regex = new RegExp(`\\b(${b})\\b`, 'g');
                result = result.replace(regex, '<span class="hl-boolean">$1</span>');
            });

            // Highlight 'result' keyword in function contracts
            result = result.replace(/\b(result)\b/g, '<span class="hl-result">$1</span>');

            // Highlight numbers
            result = result.replace(/\b(\d+(?:\.\d+)?)\b/g, '<span class="hl-number">$1</span>');

            if (commentPart) {
                result += `<span class="hl-comment">${escapeHtml(commentPart)}</span>`;
            }

            return result;
        });

        return highlightedLines.join('\n') + '\n ';
    }

    function updateLineNumbers() {
        const lines = codeEditor.value.split('\n').length;
        let lineStr = '';
        for (let i = 1; i <= lines; i++) {
            lineStr += i + '\n';
        }
        lineNumbers.textContent = lineStr;
    }

    function syncEditor() {
        const text = codeEditor.value;
        highlightLayer.innerHTML = highlightSyntax(text);
        updateLineNumbers();
        highlightLayer.scrollTop = codeEditor.scrollTop;
        highlightLayer.scrollLeft = codeEditor.scrollLeft;
        lineNumbers.scrollTop = codeEditor.scrollTop;
    }

    function updateCursorInfo() {
        const pos = codeEditor.selectionStart;
        const text = codeEditor.value.substring(0, pos);
        const lines = text.split('\n');
        const lineNum = lines.length;
        const colNum = lines[lines.length - 1].length + 1;
        cursorPosition.textContent = `Ln ${lineNum}, Col ${colNum}`;
    }

    // Editor Event Listeners
    codeEditor.addEventListener('input', () => {
        fileDirtyIndicator.classList.add('dirty');
        syncEditor();
        updateCursorInfo();
    });

    codeEditor.addEventListener('scroll', () => {
        highlightLayer.scrollTop = codeEditor.scrollTop;
        highlightLayer.scrollLeft = codeEditor.scrollLeft;
        lineNumbers.scrollTop = codeEditor.scrollTop;
    });

    codeEditor.addEventListener('click', updateCursorInfo);
    codeEditor.addEventListener('keyup', updateCursorInfo);

    // Tab key support (4 spaces) & Shortcuts (Cmd+Enter, Cmd+S)
    codeEditor.addEventListener('keydown', (e) => {
        if (e.key === 'Tab') {
            e.preventDefault();
            const start = codeEditor.selectionStart;
            const end = codeEditor.selectionEnd;
            codeEditor.value = codeEditor.value.substring(0, start) + '    ' + codeEditor.value.substring(end);
            codeEditor.selectionStart = codeEditor.selectionEnd = start + 4;
            syncEditor();
            updateCursorInfo();
        } else if ((e.metaKey || e.ctrlKey) && e.key === 'Enter') {
            e.preventDefault();
            executeCode();
        } else if ((e.metaKey || e.ctrlKey) && e.key === 's') {
            e.preventDefault();
            saveCurrentFile();
        }
    });

    // New File Function
    function createNewFile() {
        const defaultName = `new_program_${Date.now().toString().slice(-4)}.ver`;
        const name = prompt('Enter a name for the new Vera file:', defaultName);
        if (!name) return;

        let filename = name.trim();
        if (!filename.endsWith('.ver')) {
            filename += '.ver';
        }

        const template = `// ${filename}
// Verification-Aware Programming Language

fn safe_divide(a, b) requires b != 0 ensures result * b == a {
    return a / b;
}

let x = 10;
let y = 2;
assert x > y;
print safe_divide(x, y);
`;

        currentFilename.textContent = filename;
        currentActiveId = filename;
        codeEditor.value = template;
        fileDirtyIndicator.classList.remove('dirty');

        // Deselect sidebar items
        document.querySelectorAll('.example-item').forEach(el => el.classList.remove('active'));

        syncEditor();
        updateCursorInfo();
        saveCurrentFile(filename);
    }

    // Save File Function
    async function saveCurrentFile(explicitFilename) {
        let filename = explicitFilename || currentFilename.textContent.trim();
        if (!filename || filename === 'scratch.ver') {
            filename = prompt('Enter filename to save:', 'program.ver');
            if (!filename) return;
        }

        if (!filename.endsWith('.ver')) {
            filename += '.ver';
        }

        const content = codeEditor.value;

        try {
            const res = await fetch('/api/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ filename, content })
            });

            const data = await res.json();
            if (data.success) {
                currentFilename.textContent = data.filename;
                fileDirtyIndicator.classList.remove('dirty');
                systemStatus.querySelector('.status-label').textContent = `Saved ${data.filename}`;
                // Reload sidebar list
                loadExamples(data.filename);
            } else {
                alert(`Save failed: ${data.error}`);
            }
        } catch (err) {
            alert(`Error saving file: ${err.message}`);
        }
    }

    btnNew.addEventListener('click', createNewFile);
    if (btnSidebarNew) btnSidebarNew.addEventListener('click', createNewFile);
    btnSave.addEventListener('click', () => saveCurrentFile());

    // Dock Tabs Switcher
    dockTabs.forEach(tab => {
        tab.addEventListener('click', () => {
            dockTabs.forEach(t => t.classList.remove('active'));
            tabPanes.forEach(p => p.classList.remove('active'));

            tab.classList.add('active');
            const target = tab.getAttribute('data-tab');
            const pane = document.getElementById(`pane-${target}`);
            if (pane) pane.classList.add('active');
        });
    });

    // Clear Button
    btnClear.addEventListener('click', () => {
        terminalOutput.textContent = 'Cleared.';
        logsOutput.textContent = '';
        astOutput.textContent = '';
    });

    // Execute / Verify Code
    async function executeCode() {
        const code = codeEditor.value;
        if (!code.trim()) {
            terminalOutput.textContent = 'Please write some Vera code first.';
            return;
        }

        // Set status to busy
        const dot = systemStatus.querySelector('.status-dot');
        const label = systemStatus.querySelector('.status-label');
        dot.classList.add('busy');
        label.textContent = 'Verifying & Executing...';

        try {
            const res = await fetch('/api/execute', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ code })
            });

            const data = await res.json();

            // Populate Output Panes
            terminalOutput.textContent = data.output || (data.verificationFailed ? 'VM execution aborted due to static verification failure.' : '(Program finished with no output)');
            astOutput.textContent = data.ast || 'No AST generated.';
            logsOutput.textContent = data.raw || '';

            // Update Formal Verification Panel
            updateVerificationUI(data);

            dot.classList.remove('busy');
            label.textContent = data.success ? 'Success' : 'Completed (Failed)';
        } catch (err) {
            dot.classList.remove('busy');
            label.textContent = 'Server Error';
            terminalOutput.textContent = `Execution failed: ${err.message}`;
            logsOutput.textContent = err.stack || err.message;
        }
    }

    function updateVerificationUI(data) {
        statusCard.className = 'verification-card';

        if (data.verificationPass) {
            statusCard.classList.add('success');
            statusIcon.textContent = '✅';
            statusHeading.textContent = 'All Invariants & Contracts Proven';
            statusDescription.textContent = 'Z3 confirmed validity (unsat). No inputs can violate your program conditions.';
            counterexamplesSection.style.display = 'none';
        } else if (data.verificationFailed) {
            statusCard.classList.add('failure');
            statusIcon.textContent = '❌';
            statusHeading.textContent = 'Verification Refuted';
            statusDescription.textContent = 'Z3 found a counterexample. Program safety could not be guaranteed at compile time.';

            if (data.counterexamples && data.counterexamples.length > 0) {
                counterexamplesSection.style.display = 'block';
                counterexamplesContainer.innerHTML = '';
                data.counterexamples.forEach(item => {
                    const pill = document.createElement('div');
                    pill.className = 'counterexample-pill';
                    pill.innerHTML = `
                        <span class="var-name">${escapeHtml(item.name)}</span>
                        <span class="var-value">${escapeHtml(item.value)}</span>
                    `;
                    counterexamplesContainer.appendChild(pill);
                });
            } else {
                counterexamplesSection.style.display = 'none';
            }
        } else {
            statusIcon.textContent = '🛡️';
            statusHeading.textContent = 'Completed';
            statusDescription.textContent = 'Program executed without explicit static verification conditions.';
            counterexamplesSection.style.display = 'none';
        }

        // Render checks list
        if (data.checks && data.checks.length > 0) {
            checksContainer.innerHTML = '';
            data.checks.forEach(check => {
                const item = document.createElement('div');
                const isPass = check.status === 'PASS';
                item.className = `check-item ${isPass ? 'pass' : 'fail'}`;
                item.innerHTML = `
                    <span class="check-status-badge ${isPass ? 'pass' : 'fail'}">${check.status}</span>
                    <span>${escapeHtml(check.title)}</span>
                `;
                checksContainer.appendChild(item);
            });
        } else {
            checksContainer.innerHTML = '<div class="empty-state">No proof obligations recorded.</div>';
        }
    }

    btnRun.addEventListener('click', executeCode);
    btnVerify.addEventListener('click', executeCode);

    // Load Examples Library
    async function loadExamples(preferredFilename) {
        try {
            const res = await fetch('/api/examples');
            const data = await res.json();
            const examples = data.examples || [];

            examplesCount.textContent = examples.length;
            examplesContainer.innerHTML = '';

            examples.forEach((ex, idx) => {
                const item = document.createElement('div');
                item.className = 'example-item';
                const isFail = ex.filename.includes('fail');
                const tagClass = isFail ? 'tag-fail' : 'tag-ok';
                const tagText = isFail ? 'FAILS' : 'PASS';

                item.innerHTML = `
                    <span>${escapeHtml(ex.title)}</span>
                    <span class="example-tag ${tagClass}">${tagText}</span>
                `;

                item.addEventListener('click', () => {
                    document.querySelectorAll('.example-item').forEach(el => el.classList.remove('active'));
                    item.classList.add('active');
                    currentActiveId = ex.id;
                    currentFilename.textContent = ex.filename;
                    codeEditor.value = ex.content;
                    fileDirtyIndicator.classList.remove('dirty');
                    syncEditor();
                    updateCursorInfo();
                    executeCode();
                });

                examplesContainer.appendChild(item);

                // Auto-select preferred filename or default
                const shouldSelect = preferredFilename 
                    ? ex.filename === preferredFilename
                    : (!currentActiveId && (ex.filename === '14_func_post_fail.ver' || idx === 0));

                if (shouldSelect) {
                    item.classList.add('active');
                    currentActiveId = ex.id;
                    currentFilename.textContent = ex.filename;
                    codeEditor.value = ex.content;
                    fileDirtyIndicator.classList.remove('dirty');
                }
            });

            syncEditor();
            updateCursorInfo();
            if (!preferredFilename) {
                executeCode();
            }
        } catch (err) {
            examplesContainer.innerHTML = `<div class="empty-state">Failed to load examples: ${err.message}</div>`;
        }
    }

    loadExamples();
});
