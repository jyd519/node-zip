const zip = require('../');
const fs = require('fs');

test("test create", async () => {
    const z = await zip.create("./tests/temp/new.zip", "123");
    z.close();
    expect(fs.existsSync("./tests/temp/new.zip")).toBe(true);
});

test("test addDir", async () => {
    jest.setTimeout(10000);
    const zipf = './tests/temp/new1.zip';
    const z = await zip.create(zipf, "123");
    let ok = await z.addDir("native/third_party/minizip");
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open(zipf, "123");
    expect(r.exists('native/third_party/minizip/README.md')).toBe(true);
    r.close();
}, 10000);

test("test addDir with root", async () => {
    const z = await zip.create("./tests/temp/new2.zip", "123");
    let ok = await z.addDir("native/third_party/minizip", "native/third_party");
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open("./tests/temp/new2.zip", "123");
    expect(r.exists('minizip/README.md')).toBe(true);
    r.close();
});

test("test addDir with wildchar", async () => {
    const zipfile = './tests/temp/new3.zip';
    const z = await zip.create(zipfile, "123");
    let ok = await z.addDir("native/third_party/minizip/*.md");
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open(zipfile, "123");
    expect(r.exists('native/third_party/minizip/README.md')).toBe(true);
    expect(r.exists('native/third_party/minizip/mz_os.h')).toBe(false);
    r.close();
});

test("test addDir with no recursive", async () => {
    const zipfile = './tests/temp/new4.zip';
    const z = await zip.create(zipfile, "123");
    let ok = await z.addDir("native/third_party/minizip/*.*", "", false);
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open(zipfile, "123");
    expect(r.exists('native/third_party/minizip/README.md')).toBe(true);
    expect(r.exists('native/third_party/minizip/mz_os.h')).toBe(true);
    expect(r.exists('native/third_party/minizip/doc')).toBe(false);
    r.close();
});

test("test addFile", async () => {
    const z = await zip.create("./tests/temp/new-file.zip", "123");
    let ok = await z.addFile("./package.json");
    expect(ok).toBe(true);
    ok = await z.addFile("./index.js", "new-index.js");
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open("./tests/temp/new-file.zip", "123");

    {
        let d = await r.read("package.json");
        const pkg = fs.readFileSync("./package.json", {
            encoding: "utf8"
        });
        expect(d).toBe(pkg);
    }

    {
        let d = await r.read("new-index.js");
        const index = fs.readFileSync("./index.js", {
            encoding: "utf8"
        });
        expect(d).toBe(index);
    }
    r.close();
});



test("test addBuffer", async () => {
    const data = "hello, world!";
    const z = await zip.create("./tests/temp/new-file2.zip", "123");
    ok = await z.addBuffer("hello.txt", Buffer.from(data));
    expect(ok).toBe(true);
    z.close();

    const r = await zip.open("./tests/temp/new-file2.zip", "123");

    {
        let d = await r.read("hello.txt");
        expect(d).toBe(data);
    }

    r.close();
});