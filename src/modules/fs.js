/*jslint sloppy: true, nomen: true */
/*global exports:true */

/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2011 Ivan De Marino <ivan.de.marino@gmail.com>
  Copyright (C) 2012-13 execjosh, http://execjosh.blogspot.com

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

function defineProperty(o, name, value) {
    Object.defineProperty(o, name, {value: value});
}

function exportProperty(name, value) {
    defineProperty(exports, name, value);
}

// JavaScript "shim" to throw exceptions in case a critical operation fails.

/** Convert a modeOrOpts to a map
 *
 * @param modeOrOpts
 *  mode: Open Mode. A string made of 'r', 'w', 'a/+', 'b' characters.
 *  opts: Options.
 *          - mode (see Open Mode above)
 *          - charset An IANA, case insensitive, charset name.
 */
function modeOrOptsToOpts(modeOrOpts) {
    var opts;

    // Extract charset from opts
    if (modeOrOpts == null) {
        // Empty options
        opts = {};
    } else if (typeof modeOrOpts !== 'object') {
        opts = {
            mode: modeOrOpts
        };
    } else {
        opts = modeOrOpts;
    }

    return opts;
}

/** Open and return a "file" object.
 * It will throw exception if it fails.
 *
 * @param path Path of the file to open
 * @param modeOrOpts
 *  mode: Open Mode. A string made of 'r', 'w', 'a/+', 'b' characters.
 *  opts: Options.
 *          - mode (see Open Mode above)
 *          - charset An IANA, case insensitive, charset name.
 * @return "file" object
 */
exportProperty("open", function open(path, modeOrOpts) {
    // Open file
    var file = exports._open(path, modeOrOptsToOpts(modeOrOpts));
    if (file) {
        return file;
    }
    throw "Unable to open file '" + path + "'";
});

/** Open, read and return text content of a file.
 * It will throw an exception if it fails.
 *
 * @param path Path of the file to read from
 * @param modeOrOpts
 *  mode: Open Mode. 'b' to open a raw binary file
 *  opts: Options.
 *          - mode (see Open Mode above)
 *          - charset An IANA, case insensitive, charset name.
 * @return file content
 */
exportProperty("read", function read(path, modeOrOpts) {
    if (typeof modeOrOpts == 'string') {
        if (modeOrOpts.toLowerCase() == 'b') {
            // open binary
            modeOrOpts = {mode: modeOrOpts};
        } else {
            // asume charset is given
            modeOrOpts = {charset: modeOrOpts};
        }
    }
    var opts = modeOrOptsToOpts(modeOrOpts);
    // ensure we open for reading
    if ( typeof opts.mode !== 'string' ) {
        opts.mode = 'r';
    } else if ( opts.mode.indexOf('r') == -1 ) {
        opts.mode += 'r';
    }
    var f = exports.open(path, opts),
        content = f.read();

    f.close();
    return content;
});

/** Open and write text content to a file
 * It will throw an exception if it fails.
 *
 * @param path Path of the file to read from
 * @param content Content to write to the file
 * @param modeOrOpts
 *  mode: Open Mode. A string made of 'r', 'w', 'a/+', 'b' characters.
 *  opts: Options.
 *          - mode (see Open Mode above)
 *          - charset An IANA, case insensitive, charset name.
 */
exportProperty("write", function write(path, content, modeOrOpts) {
    var opts = modeOrOptsToOpts(modeOrOpts);
    // ensure we open for writing
    if ( typeof opts.mode !== 'string' ) {
        opts.mode = 'w';
    } else if ( opts.mode.indexOf('w') == -1 ) {
        opts.mode += 'w';
    }
    var f = exports.open(path, opts);

    f.write(content);
    f.close();
});

/** Return the size of a file, in bytes.
 * It will throw an exception if it fails.
 *
 * @param path Path of the file to read the size of
 * @return File size in bytes
 */
exportProperty("size", function size(path) {
    var size = exports._size(path);
    if (size !== -1) {
        return size;
    }
    throw "Unable to read file '" + path + "' size";
});

/** Copy a file.
 * It will throw an exception if it fails.
 *
 * @param source Path of the source file
 * @param destination Path of the destination file
 */
exportProperty("copy", function copy(source, destination) {
    if (!exports._copy(source, destination)) {
        throw "Unable to copy file '" + source + "' at '" + destination + "'";
    }
});

/** Copy a directory tree.
 * It will throw an exception if it fails.
 *
 * @param source Path of the source directory tree
 * @param destination Path of the destination directory tree
 */
exportProperty("copyTree", function copyTree(source, destination) {
    if (!exports._copyTree(source, destination)) {
        throw "Unable to copy directory tree '" + source + "' at '" + destination + "'";
    }
});

/** Move a file.
 * It will throw an exception if it fails.
 *
 * @param source Path of the source file
 * @param destination Path of the destination file
 */
exportProperty("move", function move(source, destination) {
    exports.copy(source, destination);
    exports.remove(source);
});

/** Removes a file.
 * It will throw an exception if it fails.
 *
 * @param path Path of the file to remove
 */
exportProperty("remove", function remove(path) {
    if (!exports._remove(path)) {
        throw "Unable to remove file '" + path + "'";
    }
});

/** Removes a directory.
 * It will throw an exception if it fails.
 *
 * @param path Path of the directory to remove
 */
exportProperty("removeDirectory", function removeDirectory(path) {
    if (!exports._removeDirectory(path)) {
        throw "Unable to remove directory '" + path + "'";
    }
});

/** Removes a directory tree.
 * It will throw an exception if it fails.
 *
 * @param path Path of the directory tree to remove
 */
exportProperty("removeTree", function removeTree(path) {
    if (!exports._removeTree(path)) {
        throw "Unable to remove directory tree '" + path + "'";
    }
});

exportProperty("touch", function touch(path) {
    exports.write(path, "", 'a');
});

// Path stuff

exportProperty("join", function join() {
    var args = [];

    if (0 < arguments.length) {
        args = args.slice.call(arguments, 0)
            // Make sure each part is a string and remove empty parts (except at begining)
            .map(function (part, idx) {
                if (null != part) {
                    var str = part.toString();
                    if ("" === str) {
                        return 0 === idx ? "" : null;
                    } else {
                        return str;
                    }
                }
            })
            // Remove empty parts
            .filter(function (part) {
                return null != part;
            });
    }

    var ret = args.join("/");

    return 0 < ret.length ? ret : ".";
});

exportProperty("split", function split(path) {
    if (typeof path !== "string") {
        return [];
    }

    return exports.fromNativeSeparators(path)
        // Collapse redundant separators
        .replace(/\/+/g, "/")
        // Remove separator at end
        .replace(/\/$/, "")
        // And split
        .split("/")
});
