var Module = {
  noInitialRun: false,
  print: function (text) { window.PimpTerminal.write(text + "\n"); },
  printErr: function (text) { window.PimpTerminal.write(text + "\n"); },
  preRun: [function () {
    var bytes = [];
    FS.init(function () {
      if (!bytes.length) {
        var value = window.prompt("Enter the requested Pimp Wyld command or number:", "");
        if (value === null) value = "q";
        bytes = Array.from(new TextEncoder().encode(value + "\n"));
      }
      return bytes.shift();
    });
  }]
};
