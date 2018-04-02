# Scanahedron

## About
Easy image scanning module for node.

Scanahedron scans images with minimal effort, but still allows for a flexible configuration.

## Installation:

```
npm install scanahedron --save
```

See development notes below for further details on the build process & dependencies.

#### Electron
If you'd like to use this module with electron, you either can enforce to build with the same node version as the electron module was built. To do so simply add the flag "-v <node_version>" to cmake-js. I.e.:
```
cmake-js -v 8.7.0
```

Or you can rebuild your electron version to match your current node version, before your npm install this package:
```
npm install electron-rebuild -g
electron-rebuild -p
```

## Usage
Scan to file with the default scanner:
```
const scanahedron = require("path-to/libscanahedron.node")
scanahedron.scanToFile(null, "test.png");
```

Scan to buffer with a specific scanner:
```
const scanahedron = require("path-to/libscanahedron.node")
const scanners = scanahedron->getScanners();
const myScanner = filterScanner(scanners);
const buffer = scanahedron.scanToBuffer(myScanner, myScanner);

console.log(buffer.width);
console.log(buffer.height);
console.log(buffer.bytesPerPixel);
console.log(buffer.pixels);
```

Scan only the first 5 cm:
```
const scanahedron = require("path-to/libscanahedron.node")
const scanners = scanahedron.getScanners();
let configuration = scanahedron.getConfiguration(scanners[0]);
configuration.toY = 50;
scanahedron.setConfiguration(scanners[0], configuration);

scanahedron.scanToFile(scanners[0], "output.png");
```

Dump the scanner's capabilities:

```
const scanahedron = require("path-to/libscanahedron.node")
const scanners = scanahedron.getScanners();
let capabilities = scanahedron.getCapabilities(scanners[0]);
console.log(capabilities);
```

## Development Notes
The project is cmake based and uses cmake-js to configure/build the node module.

### Dependencies
- libsane-dev
- libpng-dev
- libpng++-dev

### Building
Required cmake-js:
```
npm install nan --save
npm install cmake-js --save #(or install globaly with -g option)
```

Then to build in release mode just call:
```
cmake-js
```

To build in debug:
```
cmake-js -D
```


### Testing
GTest & Gmock are required for the unit tests.
Hence install the following package (apt install):
- libgtest-dev

To build with tests enabled run:
```
cmake-js --CDBUILD_TESTS=On
```

Run the test the by simply executing the build/tests file.

#### Notes
If GTest library cannot be found you have to build it:
```
cd /usr/src/gtest # (or /usr/src/googletest/googletest/)
sudo cmake CMakeLists.txt
sudo make

#copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib
```

Same for GMock:
```
cd /usr/src/gmock # (or /usr/src/googletest/googlemock/)
sudo cmake CMakeLists.txt
sudo make

#copy or symlink libgmock.a and libgmock_main.a to your /usr/lib folder
sudo cp *.a /usr/lib
```

### Credits
The project uses the FindSane.cmake file from [here](https://github.com/rose-compiler/rose/blob/master/cmake/modules/FindSane.cmake).

### Licensing
[MIT](LICENSE.md)