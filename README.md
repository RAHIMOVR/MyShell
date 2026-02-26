<h1>Simple C Shell</h1>

<p>A basic command-line shell written in C. This project is designed to learn how operating systems handle processes, memory, and input/output.</p>

<h2>Features</h2>
<ul>
    <li><strong>Command Loop:</strong> Continuously prompts for input until <code>exit</code> is typed.</li>
    <li><strong>External Commands:</strong> Runs system commands like <code>ls</code>, <code>pwd</code>, <code>date</code>, etc.</li>
    <li><strong>Signal Handling:</strong> Pressing <code>Ctrl-C</code> prints a new prompt instead of closing the shell.</li>
    <li><strong>Memory Safety:</strong> All memory is properly freed on exit. Verified with Valgrind.</li>
</ul>

<h2>Requirements</h2>
<ul>
    <li>GCC Compiler</li>
    <li>Make</li>
    <li>Valgrind (optional, for memory testing)</li>
</ul>

<h2>How to Build</h2>
<pre><code>make</code></pre>

<h2>How to Run</h2>
<pre><code>./myshell</code></pre>

<h2>Testing for Memory Leaks</h2>
<p>To verify there are no memory leaks, run:</p>
<pre><code>make valgrind</code></pre>

<div class="note">
    <h2>Current Limitations</h2>
    <ul>
        <li><code>cd</code> command is not yet supported (will be added in a future update).</li>
        <li>Pipes (<code>|</code>) and redirection (<code>&gt;</code>, <code>&lt;</code>) are not yet supported.</li>
    </ul>
</div>

<h2>License</h2>
<p>This is a learning project. Feel free to use and modify.</p>
