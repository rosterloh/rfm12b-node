rfm12b-node
===========

This module provides bindings for the [rfm12b kernel module][https://github.com/rosterloh/rfm12b-linux] to node.js. The module needs to be compiled and inserted into the kernel providing a device file for communications.

Building
--------

This module is built with node-gyp
npm install -g node-gyp
node-gyp configure
node-gyp build