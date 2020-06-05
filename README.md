# node-zip

<a target="_blank" rel="noopener noreferrer" href="https://github.com/nodejs/abi-stable-node/blob/doc/assets/N-API%20v3%20Badge.svg"><img src="https://github.com/nodejs/abi-stable-node/raw/doc/assets/N-API%20v3%20Badge.svg?sanitize=true" alt="N-API v3 Badge" style="max-width:100%;"></a>

node-zip is a node addon binding [Minizip](http://www.winimage.com/zLibDll/minizip.html) for reading/writing zip files.


## Quick Start


### Reading zip file 

```javascript
const r = await zip.open('./tests/test.zip');

r.exists('yargs/index.js');  // true

// List files in the zip
for (let i = 0; i < r.count; ++i) {
  // console.log(r.item(i));
}

// Extract all files
await r.extract_all('./tests/temp/all');

// Extract files matching the specified pattern
await r.extract_all('./tests/temp/partial', 'yargs/locales/**');

r.close();
```

### Writing zip file

```javascript
const w = await zip.create("new-file.zip", "123");
await w.addFile("package.json");
await w.addFile("index.js", "new-name-in-zip.js");

await w.addDir("native/third_party/minizip");
await w.addDir("native/third_party/minizip", "native/third_party"); // keep minzip

await w.addBuffer("hello.txt", Buffer.from("hello, world!"));

w.close();
```

## APIs

+ `zip.open(zipfile, [password]): Promise<Reader>`

    * `zipfile` String
    * `password` String

+ `zip.create(zipfile, [password]): Promise<Writer>`

    * `zipfile` string 
    * `password` String

+ `Reader Object`
   - `count: number` Number of files in the zip
   - `exists(path): boolean`
   - `item(index): FileInfo`
   - `read(path): Promise<string>`
   - `extract(path, dest): Promise<boolean>`
   - `extract_all(dest_dir, [pattern]): Promise<boolean>`
   - `close() `

+ `Writer Object`
   - `addBuffer(name, Buffer): Promise<>`
   - `addDir(dir, [pattern], recursive): Promise<>`
   - `addFile(file, [new-name]): Promise<>`
   - `close() `

## License

MIT license. 
