var Module = {
  noInitialRun: false,
  locateFile: function (path) { return path + "?build=fb2d002"; },
  print: function (text) { window.PimpTerminal.write(text + "\n"); },
  printErr: function (text) { window.PimpTerminal.write(text + "\n"); }
};
