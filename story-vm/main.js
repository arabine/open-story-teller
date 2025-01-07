Module.onRuntimeInitialized = function() {
    // Créer une instance de WebServer
    var startVm = Module.cwrap('storyvm_start', 'void', ['number', 'number']);

    console.log("Starting VM with story");
    startVm(0, 0);

    
};
