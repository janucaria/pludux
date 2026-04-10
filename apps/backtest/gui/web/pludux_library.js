mergeInto(LibraryManager.library, {
  $pluduxJsHelpers__deps: ['$UTF8ToString', '$stringToNewUTF8'],
  $pluduxJsHelpers: {
    ensureHiddenFileInput: function (elementId, accept) {
      if (document.getElementById(elementId)) {
        return;
      }

      var fileInput = document.createElement('input');
      fileInput.type = 'file';
      fileInput.id = elementId;
      fileInput.accept = accept;
      fileInput.style.display = 'none';
      document.body.appendChild(fileInput);
    },

    readFileAsText: function (file, onLoaded) {
      var reader = new FileReader();
      reader.onload = function (event) {
        var data = event.target.result;
        var decoder = new TextDecoder('utf-8');
        var decodedData = decoder.decode(data);
        onLoaded(file.name, decodedData);
      };
      reader.readAsArrayBuffer(file);
    },

    openTransientTextFileDialog: function (accept, multiple, onLoaded) {
      var fileSelector = document.createElement('input');
      fileSelector.type = 'file';
      fileSelector.accept = accept;
      fileSelector.multiple = multiple;
      fileSelector.style.display = 'none';
      document.body.appendChild(fileSelector);

      var cleanup = function () {
        fileSelector.value = '';
        fileSelector.remove();
      };

      fileSelector.onchange = function (event) {
        var files = Array.from(event.target.files || []);
        if (!files.length) {
          cleanup();
          return;
        }

        var pendingFiles = files.length;
        files.forEach(function (file) {
          pluduxJsHelpers.readFileAsText(file, function (fileName, decodedData) {
            onLoaded(fileName, decodedData);
            pendingFiles -= 1;
            if (pendingFiles === 0) {
              cleanup();
            }
          });
        });
      };

      fileSelector.click();
    }
  },

  pludux_js_ensure_hidden_file_input__deps: ['$pluduxJsHelpers'],
  pludux_js_ensure_hidden_file_input: function (elementIdPtr, acceptPtr) {
    pluduxJsHelpers.ensureHiddenFileInput(
      UTF8ToString(elementIdPtr),
      UTF8ToString(acceptPtr)
    );
  },

  pludux_js_open_managed_text_file_input__deps: ['$pluduxJsHelpers'],
  pludux_js_open_managed_text_file_input: function (elementIdPtr, userFunction) {
    var fileSelector = document.getElementById(UTF8ToString(elementIdPtr));
    if (!fileSelector) {
      return;
    }

    fileSelector.onchange = function () {
      var file = fileSelector.files[0];
      if (!file) {
        return;
      }

      pluduxJsHelpers.readFileAsText(file, function (fileName, decodedData) {
        var namePtr = stringToNewUTF8(fileName);
        var dataPtr = stringToNewUTF8(decodedData);

        Module._pludux_call_free_callback_2(userFunction, namePtr, dataPtr);
        fileSelector.value = '';
      });
    };

    fileSelector.click();
  },

  pludux_js_open_single_text_file__deps: ['$pluduxJsHelpers', '$stringToNewUTF8'],
  pludux_js_open_single_text_file: function (acceptPtr, userCallback, userData) {
    pluduxJsHelpers.openTransientTextFileDialog(
      UTF8ToString(acceptPtr),
      false,
      function (_fileName, decodedData) {
        var dataPtr = stringToNewUTF8(decodedData);
        Module._pludux_apps_backtest_js_opened_file_text_ready(
          dataPtr,
          userCallback,
          userData
        );
      }
    );
  },

  pludux_js_open_multiple_text_files__deps: ['$pluduxJsHelpers', '$stringToNewUTF8'],
  pludux_js_open_multiple_text_files: function (acceptPtr, userCallback, userData) {
    pluduxJsHelpers.openTransientTextFileDialog(
      UTF8ToString(acceptPtr),
      true,
      function (fileName, decodedData) {
        var namePtr = stringToNewUTF8(fileName);
        var dataPtr = stringToNewUTF8(decodedData);

        Module._pludux_apps_backtest_js_opened_file_content_ready(
          namePtr,
          dataPtr,
          userCallback,
          userData
        );
      }
    );
  },

  pludux_js_save_file__deps: ['$UTF8ToString'],
  pludux_js_save_file: function (fileNamePtr, contentPtr, mimeTypePtr) {
    var fileName = UTF8ToString(fileNamePtr);
    var content = UTF8ToString(contentPtr);
    var mimeType = UTF8ToString(mimeTypePtr);

    var fallbackDownload = function () {
      var fileSave = document.createElement('a');
      fileSave.download = fileName;
      fileSave.style.display = 'none';

      var data = new Blob([content], { type: mimeType });
      fileSave.href = URL.createObjectURL(data);

      document.body.appendChild(fileSave);
      fileSave.click();
      URL.revokeObjectURL(fileSave.href);
      fileSave.remove();
    };

    if (!window.showSaveFilePicker) {
      fallbackDownload();
      return;
    }

    var dotIndex = fileName.lastIndexOf('.');
    var extension = dotIndex >= 0 ? fileName.slice(dotIndex) : '';

    (async function () {
      try {
        var pickerOptions = {
          suggestedName: fileName,
          excludeAcceptAllOption: false,
          types: [{
            description: 'Pludux export',
            accept: {
              [mimeType || 'application/octet-stream']: extension ? [extension] : []
            }
          }]
        };

        var handle = await window.showSaveFilePicker(pickerOptions);
        var writable = await handle.createWritable();
        await writable.write(new Blob([content], { type: mimeType }));
        await writable.close();
      } catch (error) {
        // AbortError is user-cancel. Fallback keeps behavior on unsupported/blocked cases.
        if (error && error.name === 'AbortError') {
          return;
        }
        fallbackDownload();
      }
    })();
  },

  pludux_js_is_file_system_access_supported: function () {
    return window.showSaveFilePicker ? 1 : 0;
  },

  pludux_js_set_body_cursor: function (isPointer) {
    document.body.style.cursor = isPointer ? 'pointer' : 'default';
  },

  pludux_js_open_url__deps: ['$UTF8ToString'],
  pludux_js_open_url: function (urlPtr) {
    window.open(UTF8ToString(urlPtr), '_blank');
  }
});