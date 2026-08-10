(() => {
  "use strict";
  const manifest = window.ORCHESTRATOR_DOCS;
  const nav = document.querySelector("nav");
  const input = document.querySelector("input[type=search]");
  const count = document.querySelector(".count");
  const status = document.querySelector(".atlas-status");
  const frame = document.querySelector("iframe");
  const empty = document.querySelector(".empty");
  if (!manifest || !Array.isArray(manifest.entries)) {
    empty.textContent = "The checked-in documentation manifest could not be loaded.";
    return;
  }
  status.textContent = manifest.status;

  function select(entry) {
    frame.src = entry.page;
    frame.style.display = "block";
    empty.style.display = "none";
    location.hash = encodeURIComponent(entry.page);
    document.querySelectorAll("nav a").forEach(link => link.classList.toggle("active", link.dataset.page === entry.page));
  }

  function render(query = "") {
    const normalized = query.trim().toLowerCase();
    const matches = manifest.entries.filter(entry => !normalized || entry.search.includes(normalized));
    nav.replaceChildren();
    const categories = new Map();
    matches.forEach(entry => {
      if (!categories.has(entry.category)) categories.set(entry.category, []);
      categories.get(entry.category).push(entry);
    });
    categories.forEach((entries, category) => {
      const heading = document.createElement("h2");
      heading.textContent = category;
      nav.append(heading);
      entries.forEach(entry => {
        const link = document.createElement("a");
        link.href = entry.page;
        link.target = "reference";
        link.dataset.page = entry.page;
        link.textContent = entry.title;
        const kind = document.createElement("small");
        kind.textContent = `${entry.kind} · ${entry.status}`;
        link.append(kind);
        link.addEventListener("click", event => {
          event.preventDefault();
          select(entry);
        });
        nav.append(link);
      });
    });
    count.textContent = `${matches.length} of ${manifest.entries.length} entries`;
  }

  input.addEventListener("input", () => render(input.value));
  render();
  const requested = decodeURIComponent(location.hash.slice(1));
  const initial = manifest.entries.find(entry => entry.page === requested) || manifest.entries[0];
  if (initial) select(initial);
})();
