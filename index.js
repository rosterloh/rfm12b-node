"use strict";

var RadioBinding = require("bindings")("rfm12b.node");
var EventEmitter = require('events').EventEmitter;
var util = require('util');
var stream = require('stream');

function RadioFactory() {
    
    var factory = this;
    
    var _settings = {
        groupID: 211,
        bandID: 2,
        bitRate: 0x06,
        sendACK: true,
        nodeID: 12
    };
    
    function SerialPort(path, settings, callback) {

        var self = this;
        
        var args = Array.prototype.slice.call(arguments);
        callback = args.pop();
        if (typeof(callback) !== 'function') {
            callback = null;
        }
        
        stream.Stream.call(this);
        
        callback = callback || function (err) {
            if (err) {
                factory.emit('error', err);
            }
        };
        
        var err;
        
        // Check for number between 0 and 250
        settings.groupID = settings.groupID || _settings.groupID;
        if ((settings.groupID < 0) || (settings.groupID > 250)) {
            err = new Error('Invalid "groupID": ' + settings.groupID);
            callback(err);
            return;
        }

    options.stopBits = options.stopBits || options.stopbits || _options.stopbits;
    if (STOPBITS.indexOf(options.stopBits) === -1) {
      err = new Error('Invalid "stopbits": ' + options.stopbits);
      callback(err);
      return;
    }

    options.parity = options.parity || _options.parity;
    if (PARITY.indexOf(options.parity) === -1) {
      err = new Error('Invalid "parity": ' + options.parity);
      callback(err);
      return;
    }
    if (!path) {
      err = new Error('Invalid port specified: ' + path);
      callback(err);
      return;
    }

    // flush defaults, then update with provided details
    options.rtscts = _options.rtscts;
    options.xon = _options.xon;
    options.xoff = _options.xoff;
    options.xany = _options.xany;

    if (options.flowControl || options.flowcontrol) {
      var fc = options.flowControl || options.flowcontrol;

      if (typeof fc === 'boolean') {
        options.rtscts = true;
      } else {
        fc.forEach(function (flowControl) {
          var fcup = flowControl.toUpperCase();
          var idx = FLOWCONTROLS.indexOf(fcup);
          if (idx < 0) {
            var err = new Error('Invalid "flowControl": ' + fcup + ". Valid options: " + FLOWCONTROLS.join(", "));
            callback(err);
            return;
          } else {

            // "XON", "XOFF", "XANY", "DTRDTS", "RTSCTS"
            switch (idx) {
              case 0: options.xon = true; break;
              case 1: options.xoff = true; break;
              case 2: options.xany = true;  break;
              case 3: options.rtscts = true; break;
            }
          }
        });
      }
    }

    options.bufferSize = options.bufferSize || options.buffersize || _options.buffersize;
    options.parser = options.parser || _options.parser;

    options.dataCallback = options.dataCallback || function (data) {
      options.parser(self, data);
    };

    options.disconnectedCallback = options.disconnectedCallback || function () {
      if (self.closing) {
        return;
      }
      var err = new Error("Disconnected");
      callback(err);
    };

    if (process.platform !== 'win32') {
      // All other platforms:
      this.fd = null;
      this.paused = true;
      this.bufferSize = options.bufferSize || 64 * 1024;
      this.readable = true;
      this.reading = false;

      if (options.encoding) {
        this.setEncoding(this.encoding);
      }
    }

    this.options = options;
    this.path = path;

    if (openImmediately) {
      process.nextTick(function () {
        self.open(callback);
      });
    }
  }

  util.inherits(SerialPort, stream.Stream);
  
}