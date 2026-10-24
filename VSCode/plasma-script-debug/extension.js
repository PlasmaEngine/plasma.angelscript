// Connects VS Code's debugger to the script debug adapter that the Plasma editor runs on 127.0.0.1.
const vscode = require('vscode');

function activate(context) {
  context.subscriptions.push(
    vscode.debug.registerDebugAdapterDescriptorFactory('plasma', {
      createDebugAdapterDescriptor(session) {
        const port = session.configuration.port || 4712;
        return new vscode.DebugAdapterServer(port, '127.0.0.1');
      },
    })
  );
}

function deactivate() {}

module.exports = { activate, deactivate };
