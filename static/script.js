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
