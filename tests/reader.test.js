const zip = require('../');
const fs = require('fs');
const rimraf = require("rimraf");

test("open zip", async () => {
    let z = await zip.open('./tests/test.zip');
    expect(z.count).toBeGreaterThan(1);
    z.close();
});

test("open zip with standard encryption", async () => {
    let z = await zip.open('./tests/test-encrypted.zip', '123');
    expect(z.count).toBeGreaterThan(1);
    z.close();
});


test("open zip with aes256", async () => {
    let z = await zip.open('./tests/test-aes256.zip', '123');
    expect(z.count).toBeGreaterThan(1);
    z.close();
});


test("open zip not exists", async () => {
    try {
        var z = await zip.open('./tests/not-exist.zip');
    } catch (e) {
        expect(e.message).toMatch('code: -111');
    }
});

test("test exists", async () => {
    var z = await zip.open('./tests/test-aes256.zip', '123');
    expect(z.exists('yargs/index.js')).toBe(true);
    expect(z.exists('yargs/index.js')).toBe(true);
    expect(z.exists('xxx.json')).toBe(false);
    z.close();
});

test("test item", async () => {
    var z = await zip.open('./tests/test-aes256.zip', '123');
    for (let i = 0; i < z.count; ++i) {
        // console.log(z.item(i));
        expect(z.item(i).name.length).toBeGreaterThan(0);
    }
    z.close();
});

test("test extract", async () => {
    var z = await zip.open('./tests/test.zip');
    rimraf.sync("./tests/temp");

    let n = await z.extract_all('./tests/temp/all');
    expect(n).toBeGreaterThan(10);
    expect(fs.existsSync('./tests/temp/all/yargs/index.js')).toBe(true);

    n = await z.extract_all('./tests/temp/partial', 'yargs/locales/**');
    expect(n).toBeGreaterThan(10);
    expect(fs.existsSync('./tests/temp/partial/yargs/locales/en.json')).toBe(true);

    z.close();
});


test("test read", async () => {
    var z = await zip.open('./tests/test-aes256.zip', '123');
    const data = await z.read("yargs/index.js");
    const expected = "var assert = require('assert')";
    expect(data.substr(0, expected.length)).toBe(expected);
});


test("test read simultaneously", async () => {
    var z = await zip.open('./tests/test-aes256.zip', '123');
    const files = [
        "yargs/index.js",
        "yargs/README.md",
        "yargs/CHANGELOG.md",
    ];
    const data = await Promise.all([
        z.read(files[0]),
        z.read(files[1]),
        z.read(files[2]),
    ]);

    const dir = `./tests/temp/all/`;
    files.forEach((name, i) => {
        const f1 = fs.readFileSync(dir + name, { encoding: "utf8" });
        expect(data[i]).toBe(f1);
    });
});
