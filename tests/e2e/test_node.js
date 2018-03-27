const scanahedron = require("../../build/Release/libscanahedron.node")

const scanners = scanahedron.getScanners();
console.log(scanners);

const capabilties = scanahedron.getCapabilities(scanners[0]);
console.log(capabilties);

let configuration = scanahedron.getConfiguration(scanners[0]);
console.log(configuration);
configuration.fromY = 0;
configuration.toY = 5;
configuration.resolution = capabilties.possibleResolutionsInDPI[0];
scanahedron.setConfiguration(scanners[0], configuration);

scanahedron.scanToFile(scanners[0], "test.png");

const buffer = scanahedron.scanToBuffer(scanners[0]);

console.log(buffer.width);
console.log(buffer.height);
console.log(buffer.bytesPerPixel);
console.log(buffer.pixels);
