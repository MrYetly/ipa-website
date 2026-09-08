//htmx.logAll();
(() => {
  // A Map to keep track of elements we've already observed 
  // This prevents memory leaks and attaching multiple observers to one element
  const observedElements = new WeakMap();

  function initializeResizeObservers(parentElement) {
    // Find all elements that need resize watching within the target area
    const targets = parentElement.querySelectorAll('.watch-resize');

    targets.forEach(target => {
      // Skip if we are already watching this specific element
      if (observedElements.has(target)) return;

      const observer = new ResizeObserver(() => {
        // Dispatch the custom event htmx listens for
	      console.log("element-resize:", target);
        target.dispatchEvent(new CustomEvent('element-resize'));
      });

      observer.observe(target);
      
      // Save it to our map so we don't double-observe it later
      observedElements.set(target, observer);
    });
  }

  // 1. Run on initial page load
  document.addEventListener('DOMContentLoaded', () => {
    initializeResizeObservers(document.body);
  });

  // 2. Run every time htmx swaps and processes new HTML content
  document.addEventListener('htmx:afterSettle', (event) => {
    // event.target is the specific chunk of HTML htmx just brought in
    initializeResizeObservers(event.target);
  });
})();

// Background manager: preload text once, create DOM nodes on demand
(() => {
  const container = document.getElementById('ascii-art-backgrounds');
  if (!container) return;

  let bgLoaded = false;
  let bgLoading = false;
  let pendingBg = null;
  let walkInterval = null;
  let currentX = 0;
  let currentY = 0;
  const bgCache = {};

  const PLAY_COUNT = 8;
  const BACKGROUNDS = [
    { id: 'bg-work',          url: '/ascii-art/work' },
    { id: 'bg-this-website',  url: '/ascii-art/this-website' },
    { id: 'bg-my-interests',  url: '/ascii-art/my-interests' },
  ];
  for (let i = 0; i < PLAY_COUNT; i++) {
    BACKGROUNDS.push({ id: `bg-play-${i}`, url: `/ascii-art/play/${i}` });
  }

  function loadAllBackgrounds() {
    if (bgLoading || bgLoaded) return;
    bgLoading = true;

    let loadedCount = 0;
    BACKGROUNDS.forEach(bg => {
      fetch(bg.url)
        .then(r => r.text())
        .then(text => {
          bgCache[bg.id] = text;
        })
        .catch(err => console.error('Failed to load background:', bg.url, err))
        .finally(() => {
          loadedCount++;
          if (loadedCount === BACKGROUNDS.length) {
            bgLoaded = true;
            if (pendingBg) {
              activateBg(pendingBg);
              pendingBg = null;
            }
          }
        });
    });
  }

  function ensureElement(id) {
    let el = document.getElementById(id);
    if (!el && bgCache[id]) {
      el = document.createElement('pre');
      el.id = id;
      el.className = 'ascii-art-portrait';
      el.textContent = bgCache[id];
      container.appendChild(el);
    }
    return el;
  }

  function startWalk(el) {
    if (walkInterval) clearInterval(walkInterval);
    currentX = 0;
    currentY = 0;
    el.style.transform = '';
    pickTarget(el);
    walkInterval = setInterval(() => pickTarget(el), 8000);
  }

  function stopWalk() {
    if (walkInterval) {
      clearInterval(walkInterval);
      walkInterval = null;
    }
    currentX = 0;
    currentY = 0;
  }

  function pickTarget(el) {
    const maxY = -(el.scrollHeight - window.innerHeight);
    const safeMaxY = Math.min(0, maxY);
    const dx = (Math.random() * 4.0 - 2.0).toFixed(2);
    const dy = (Math.random() * -160.0 + 80.0).toFixed(2);
    currentX = Math.max(-5, Math.min(0, currentX + parseFloat(dx)));
    currentY = Math.max(safeMaxY, Math.min(0, currentY + parseFloat(dy)));
    el.style.transform = `translate(${currentX.toFixed(2)}vw, ${currentY.toFixed(2)}px)`;
  }

  window.activateBg = function(id) {
    if (!bgLoaded) {
      loadAllBackgrounds();
      pendingBg = id;
      return;
    }

    let targetId = id;
    if (id === 'bg-play') {
      const idx = Math.floor(Math.random() * PLAY_COUNT);
      targetId = `bg-play-${idx}`;
    }

    const outgoing = document.querySelector('.ascii-art-portrait.bg-active');

    if (outgoing) {
      outgoing.classList.remove('bg-active');
      outgoing.classList.remove('code-drift');
      outgoing.style.transform = '';
      // Remove from DOM after fade-out transition completes
      setTimeout(() => {
        if (!outgoing.classList.contains('bg-active') && outgoing.parentNode) {
          outgoing.parentNode.removeChild(outgoing);
        }
      }, 500);
    }
    stopWalk();

    const target = ensureElement(targetId);
    if (target) {
      target.classList.add('bg-active');
      if (id === 'bg-this-website') {
        target.classList.add('code-drift');
        startWalk(target);
      }
    }
  };
})();
