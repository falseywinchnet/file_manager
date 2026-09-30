'use strict';
/** @typedef {{id: string, summary: string, header: string,
 * page: string, declaration: string}} ApiSymbol */
/** @type {typeof import('node:assert/strict')} */
const assert = require('node:assert/strict');
/** @type {typeof import('node:fs')} */
const fs = require('node:fs');
/** @type {typeof import('node:path')} */
const path = require('node:path');
/** @type {typeof import('node:vm')} */
const vm = require('node:vm');

class FixtureElement {
    constructor() {
        /** @type {Array<FixtureElement>} */
        this.children = [];
        /** @type {Map<string, Function>} */
        this.listeners = new Map();
        /** @type {string} */
        this.value = '';
        /** @type {string} */
        this.textContent = '';
        /** @type {boolean} */
        this.hidden = false;
    }
    /** @param {FixtureElement} child */
    append(child) {
        this.children.push(child);
    }
    replaceChildren() {
        this.children.length = 0;
    }
    /** @param {string} name @param {Function} listener */
    addEventListener(name, listener) {
        this.listeners.set(name, listener);
    }
    /** @param {string} name @param {Function} listener */
    removeEventListener(name, listener) {
        assert.equal(this.listeners.get(name), listener);
        this.listeners.delete(name);
    }
}
class FixtureDocument {
    constructor() {
        /** @type {Map<string, FixtureElement>} */
        this.elements = new Map();
        this.elements.set('api-search', new FixtureElement());
        this.elements.set('search-results', new FixtureElement());
        this.elements.set('page-content', new FixtureElement());
    }
    /** @param {string} name @returns {FixtureElement} */
    getElementById(name) {
        /** @type {FixtureElement} */
        /** @type {FixtureElement} */        const element = this.elements.get(name);
        return element;
    }
    /** @param {string} name @returns {FixtureElement} */
    createElement(name) {
        /** @type {FixtureElement} */        const element = new FixtureElement();
        element.tagName = name;
        return element;
    }
}

function main() {
    /** @type {FixtureDocument} */    const document = new FixtureDocument();
    /** @type {FixtureElement} */    const window = new FixtureElement();
    /** @type {Array<ApiSymbol>} */
    window.GUIFormsSearch = [];
    /** @type {number} */    let index = 0;
    for (index = 0; index < 120; index += 1) {
        window.GUIFormsSearch.push({id: 'Fixture' + index + '|void()', summary: 'alpha beta',
            header: 'example.hpp', page: index + '.html', declaration: 'void fixture();'});
    }
    window.GUIFormsSearch.push({id: 'Other|int()', summary: 'alpha only',
        header: 'other.hpp', page: 'other.html', declaration: 'int other();'});
    /** @type {import('node:vm').Context} */    const context = vm.createContext({document: document, window: window,
        location: {search: '?q=ALPHA+beta'}, URLSearchParams: URLSearchParams});
    /** @type {string} */    const scriptPath = path.join(__dirname, '../tools/api_reference_search.js');
    /** @type {string} */    const source = fs.readFileSync(scriptPath, 'utf8');
    vm.runInContext(source, context);
    /** @type {{dispose: Function}} */    const owner = vm.runInContext('referenceSearch', context);
    /** @type {FixtureElement} */    const results = document.getElementById('search-results');
    /** @type {FixtureElement} */    const box = document.getElementById('api-search');
    assert.equal(results.children[0].textContent, '120 matching declarations (first 100 shown)');
    assert.equal(results.children[1].children.length, 100);
    for (index = 0; index < 100; index += 1) {
        assert.equal(results.children[1].children[index].children[0].href, index + '.html');
    }
    box.value = 'other';
    box.listeners.get('input')();
    assert.equal(results.children[0].textContent, '1 matching declarations');
    assert.equal(results.children[1].children[0].children[0].href, 'other.html');
    box.value = 'no matching symbol';
    box.listeners.get('input')();
    assert.equal(results.children[0].textContent, '0 matching declarations');
    box.value = '  ';
    box.listeners.get('input')();
    assert.equal(results.children.length, 0);
    assert.equal(document.getElementById('page-content').hidden, false);
    assert.equal(results.hidden, true);
    window.listeners.get('pagehide')();
    assert.equal(box.listeners.size, 0);
    window.listeners.get('pageshow')();
    assert.equal(box.listeners.size, 1);
    owner.dispose();
    assert.equal(box.listeners.size, 0);
    assert.equal(window.listeners.size, 0);
    console.log('Search order, 100-result bound, empty/no-match states and listener revocation passed');
}
main();
