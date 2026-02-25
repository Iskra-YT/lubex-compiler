---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

**Describe the bug**
A clear and concise description of the bug in the Lubex Compiler.
For example: "IRStruct node fails to generate correct IR for class members."

**To Reproduce**
Steps to reproduce the behavior in code:
1. Write a minimal code snippet that triggers the issue.
2. Compile or parse the snippet using Lubex Compiler.
3. Observe the error.
4. See incorrect behavior or exception.

**Expected behavior**
A clear description of what you expected to happen.
For example: "Lexer should generate DelimiterToken for ';' symbol."

**Actual behavior**
Describe what actually happens.
For example: "Compiler throws Syntax Error: Undefined variable."

**Code snippet / Example**
Include minimal code example that triggers the bug, if possible:
```lubex
class MyClass {
    var x: Int = 0
}
let obj = MyClass()
obj.x = 42
```
