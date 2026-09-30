'use strict';

/** @typedef {{id: string, summary: string, header: string,
 * page: string, declaration: string}} ApiSymbol */

/** Search state belongs to this document. Input is disconnected on pagehide;
 * document-lifetime listeners remain until disposal. Pageshow restores input
 * after a back/forward-cache return. */
class ReferenceSearch {
    constructor() {
        /** @type {HTMLInputElement} */
        this.box = document.getElementById('api-search');
        /** @type {HTMLElement} */
        this.results = document.getElementById('search-results');
        /** @type {HTMLElement} */
        this.content = document.getElementById('page-content');
        /** @type {Array<ApiSymbol>} */
        this.visibleMatches = [];
        /** @type {Array<ApiSymbol>} Read-only generated records, document lifetime. */
        this.symbols = [];
        /** @type {Array<string>} One prepared search string per symbol. */
        this.searchText = [];
        /** @type {boolean} */
        this.connected = false;
        /** @type {EventListener} Retains this owner until disconnect. */
        this.inputListener = this.search.bind(this);
        /** @type {EventListener} Document-lifetime listener. */
        this.hideListener = this.disconnect.bind(this);
        /** @type {EventListener} Document-lifetime listener. */
        this.showListener = this.connect.bind(this);
    }

    /** @returns {void} */
    start() {
        /** @type {URLSearchParams} */
        const parameters = new URLSearchParams(location.search);
        /** @type {string} */
        const query = parameters.get('q') || '';
        this.box.value = query;
        this.prepareEntries();
        window.addEventListener('pagehide', this.hideListener);
        window.addEventListener('pageshow', this.showListener);
        this.connect();
    }

    /** Prepare stable generated data before any input listener is connected.
     * @returns {void} */
    prepareEntries() {
        this.symbols = window.GUIFormsSearch.slice();
        this.searchText.length = 0;
        /** @type {number} */
        let index = 0;
        for (index = 0; index < this.symbols.length; index += 1) {
            /** @type {ApiSymbol} */
            const symbol = this.symbols[index];
            /** @type {string} */
            const combined = symbol.id + ' ' + symbol.summary + ' ' + symbol.header;
            this.searchText.push(combined.toLowerCase());
        }
    }

    /** @returns {void} */
    connect() {
        if (!this.connected) {
            this.box.addEventListener('input', this.inputListener);
            this.connected = true;
        }
        this.search();
    }

    /** @returns {void} */
    disconnect() {
        if (this.connected) {
            this.box.removeEventListener('input', this.inputListener);
            this.connected = false;
        }
    }

    /** @returns {void} */
    dispose() {
        this.disconnect();
        window.removeEventListener('pagehide', this.hideListener);
        window.removeEventListener('pageshow', this.showListener);
        this.visibleMatches.length = 0;
        this.symbols.length = 0;
        this.searchText.length = 0;
    }

    /** @returns {void} */
    search() {
        /** @type {string} */
        const lowercase = this.box.value.toLowerCase();
        /** @type {string} */
        const query = lowercase.trim();
        this.results.replaceChildren();
        this.content.hidden = Boolean(query);
        this.results.hidden = !query;
        this.visibleMatches.length = 0;
        if (!query) {
            return;
        }
        /** @type {Array<string>} */
        const words = query.split(/\s+/);
        /** @type {number} */
        let matchCount = 0;
        /** @type {number} */
        let index = 0;
        for (index = 0; index < this.symbols.length; index += 1) {
            /** @type {ApiSymbol} */
            const symbol = this.symbols[index];
            /** @type {string} */
            const searchable = this.searchText[index];
            /** @type {boolean} */
            let matches = true;
            /** @type {number} */
            let wordIndex = 0;
            for (wordIndex = 0; wordIndex < words.length; wordIndex += 1) {
                if (!searchable.includes(words[wordIndex])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                matchCount += 1;
                if (this.visibleMatches.length < 100) {
                    this.visibleMatches.push(symbol);
                }
            }
        }
        /** @type {HTMLElement} */
        const count = document.createElement('p');
        /** @type {string} */
        let countText = matchCount + ' matching declarations';
        if (matchCount > 100) {
            countText += ' (first 100 shown)';
        }
        count.textContent = countText;
        this.results.append(count);
        /** @type {HTMLElement} */
        const list = document.createElement('ul');
        list.className = 'search-results';
        for (index = 0; index < this.visibleMatches.length; index += 1) {
            /** @type {ApiSymbol} */
            const symbol = this.visibleMatches[index];
            /** @type {HTMLElement} */
            const item = document.createElement('li');
            /** @type {HTMLAnchorElement} */
            const anchor = document.createElement('a');
            anchor.href = symbol.page;
            anchor.textContent = symbol.id.split('|')[0];
            item.append(anchor);
            /** @type {HTMLElement} */
            const code = document.createElement('code');
            code.textContent = symbol.declaration;
            item.append(code);
            if (symbol.summary) {
                /** @type {HTMLElement} */
                const paragraph = document.createElement('p');
                paragraph.textContent = symbol.summary;
                item.append(paragraph);
            }
            list.append(item);
        }
        this.results.append(list);
    }
}

/** @type {ReferenceSearch} */
const referenceSearch = new ReferenceSearch();
referenceSearch.start();
