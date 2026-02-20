(function() {
  var playerName = document.getElementById('playerName');
  var workshopId = document.getElementById('workshopId');
  var workshopStatus = document.getElementById('workshopStatus');
  var modsGrid = document.getElementById('modsGrid');
  var workshopDownloadBtn = document.getElementById('workshopDownloadBtn');
  var workshopProgress = document.getElementById('workshopProgress');
  var workshopProgressFill = document.getElementById('workshopProgressFill');
  var progressFooter = document.getElementById('progressFooter');
  var progressInfo = document.getElementById('progressInfo');
  var progressFill = document.getElementById('progressFill');
  var progressPercent = document.getElementById('progressPercent');
  var progressOpenFolder = document.getElementById('progressOpenFolder');
  var _workshopDownloadFolder = '';
  var manageInstallPopup = document.getElementById('manageInstallPopup');
  var launchOptionCards = document.querySelectorAll('#launchDropdown .launch-option-card');
  var modsPagination = document.getElementById('modsPagination');
  var modsPrevPageBtn = document.getElementById('modsPrevPageBtn');
  var modsNextPageBtn = document.getElementById('modsNextPageBtn');
  var modsPageLabel = document.getElementById('modsPageLabel');
  var modsItemsCache = [];
  var modsPage = 1;
  var modsPageSize = 12;

  var workshopBrowseGrid = document.getElementById('workshopBrowseGrid');
  var workshopSearchInput = document.getElementById('workshopSearchInput');
  var workshopSearchBtn = document.getElementById('workshopSearchBtn');
  var workshopBrowseRefreshBtn = document.getElementById('workshopBrowseRefreshBtn');
  var workshopBrowseClearBtn = document.getElementById('workshopBrowseClearBtn');
  var workshopBrowsePagination = document.getElementById('workshopBrowsePagination');
  var workshopBrowseItems = [];
  var workshopBrowseCurrentPage = 1;
  var workshopBrowseItemsPerPage = 10;
  var workshopBrowseLoading = false;
  var workshopBrowseSearchTerm = '';
  var workshopBrowseCacheKey = 'workshopBrowseCache';
  var _workshopBrowsePollInterval = null;

  var workshopOverlay = document.getElementById('workshopOverlay');
  var workshopModal = document.getElementById('workshopModal');
  var workshopModalTitle = document.getElementById('workshopModalTitle');
  var workshopModalClose = document.getElementById('workshopModalClose');
  var workshopModalImage = document.getElementById('workshopModalImage');
  var workshopModalInfo = document.getElementById('workshopModalInfo');
  var workshopModalDescription = document.getElementById('workshopModalDescription');
  var workshopModalViewBtn = document.getElementById('workshopModalViewBtn');
  var workshopModalDownloadBtn = document.getElementById('workshopModalDownloadBtn');
  var workshopFilterSelect = document.getElementById('workshopFilterSelect');

  if (!Array.prototype.find) {
    Array.prototype.find = function(predicate) {
      if (this == null) throw new TypeError('Array.prototype.find called on null or undefined');
      if (typeof predicate !== 'function') throw new TypeError(predicate + ' is not a function');
      var list = Object(this);
      var length = parseInt(list.length, 10) || 0;
      var thisArg = arguments.length > 1 ? arguments[1] : void undefined;
      for (var i = 0; i < length; i++) {
        if (predicate.call(thisArg, list[i], i, list)) return list[i];
      }
      return undefined;
    };
  }

  if (!Element.prototype.closest) {
    Element.prototype.closest = function(selector) {
      var el = this;
      while (el && el.nodeType === 1) {
        if (el.matches && el.matches(selector)) return el;
        if (el.msMatchesSelector && el.msMatchesSelector(selector)) return el;
        el = el.parentElement;
      }
      return null;
    };
  }

  function enableClipboard(element) {
    if (!element) return;
    element.oncopy = null;
    element.oncut = null;
    element.onpaste = null;
    element.setAttribute('unselectable', 'off');
    element.onselectstart = function() { return true; };
    element.style.userSelect = 'text';
    element.style.webkitUserSelect = 'text';
    element.style.MozUserSelect = 'text';
    element.style.msUserSelect = 'text';
    element.style.cursor = 'text';
    if (!element.hasAttribute('tabindex')) element.setAttribute('tabindex', '0');
  }

  function escapeHtml(str) {
    if (!str) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function convertBBCodeToHtml(str) {
    if (!str) return '';
    var s = escapeHtml(str);
    s = s.replace(/\[h1\](.*?)\[\/h1\]/gi, '<h3 style="margin:6px 0 4px;font-size:1.1rem;">$1</h3>');
    s = s.replace(/\[h2\](.*?)\[\/h2\]/gi, '<h4 style="margin:5px 0 3px;font-size:1rem;">$1</h4>');
    s = s.replace(/\[h3\](.*?)\[\/h3\]/gi, '<h5 style="margin:4px 0 2px;font-size:0.95rem;">$1</h5>');
    s = s.replace(/\[b\](.*?)\[\/b\]/gi, '<strong>$1</strong>');
    s = s.replace(/\[u\](.*?)\[\/u\]/gi, '<u>$1</u>');
    s = s.replace(/\[i\](.*?)\[\/i\]/gi, '<em>$1</em>');
    s = s.replace(/\[strike\](.*?)\[\/strike\]/gi, '<s>$1</s>');
    s = s.replace(/\[code\](.*?)\[\/code\]/gi, '<code style="background:rgba(0,0,0,0.3);padding:2px 5px;border-radius:3px;">$1</code>');
    s = s.replace(/\[url=(.*?)\](.*?)\[\/url\]/gi, '<a href="$1" style="color:#5b9bd5;text-decoration:underline;cursor:pointer;" onclick="try{window.external.openUrl(this.href)}catch(e){};return false;">$2</a>');
    s = s.replace(/\[url\](.*?)\[\/url\]/gi, '<a href="$1" style="color:#5b9bd5;text-decoration:underline;cursor:pointer;" onclick="try{window.external.openUrl(this.href)}catch(e){};return false;">$1</a>');
    s = s.replace(/\[img\](.*?)\[\/img\]/gi, '<img src="$1" style="max-width:100%;border-radius:4px;margin:4px 0;" onerror="this.style.display=\'none\'">');
    s = s.replace(/\[list\]/gi, '<ul style="margin:4px 0;padding-left:20px;">');
    s = s.replace(/\[\/list\]/gi, '</ul>');
    s = s.replace(/\[\*\]/gi, '<li>');
    s = s.replace(/\[hr\]/gi, '<hr style="border:none;border-top:1px solid rgba(200,196,192,0.15);margin:6px 0;">');
    s = s.replace(/\[spoiler\](.*?)\[\/spoiler\]/gi, '<span style="background:rgba(200,196,192,0.2);padding:1px 4px;border-radius:2px;">$1</span>');
    s = s.replace(/\[noparse\](.*?)\[\/noparse\]/gi, '$1');
    s = s.replace(/\[previewyoutube=(\w+)[^\]]*\].*?\[\/previewyoutube\]/gi, '<a href="https://youtube.com/watch?v=$1" style="color:#5b9bd5;text-decoration:underline;cursor:pointer;" onclick="try{window.external.openUrl(this.href)}catch(e){};return false;">[YouTube Video]</a>');
    s = s.replace(/\r?\n/g, '<br>');
    return s;
  }

  function stripBBCode(str) {
    if (!str) return '';
    return str.replace(/\[\/?[^\]]+\]/g, '').replace(/\s+/g, ' ').replace(/^\s+|\s+$/g, '');
  }

  function saveWorkshopCache(items) {
    try {
      localStorage.setItem(workshopBrowseCacheKey, JSON.stringify({ timestamp: Date.now(), items: items }));
    } catch (e) {}
  }

  function loadWorkshopCache() {
    try {
      var cached = localStorage.getItem(workshopBrowseCacheKey);
      if (!cached) return null;
      var data = JSON.parse(cached);
      return data.items || null;
    } catch (e) { return null; }
  }

  function getExternal() {
    try { return window.external; } catch (e) { return null; }
  }

  enableClipboard(playerName);
  enableClipboard(workshopId);
  enableClipboard(workshopSearchInput);

  var gearBtn = document.getElementById('gearBtn');
  var launchDropdown = document.getElementById('launchDropdown');

  if (gearBtn && launchDropdown) {
    gearBtn.onclick = function(e) {
      e.stopPropagation();
      var isOpen = launchDropdown.classList.contains('open');
      if (isOpen) {
        launchDropdown.classList.remove('open');
        gearBtn.classList.remove('open');
      } else {
        launchDropdown.classList.add('open');
        gearBtn.classList.add('open');
      }
    };
    launchDropdown.onclick = function(e) { e.stopPropagation(); };
  }

  document.addEventListener('click', function() {
    if (launchDropdown && launchDropdown.classList.contains('open')) {
      launchDropdown.classList.remove('open');
      if (gearBtn) gearBtn.classList.remove('open');
    }
  });

  try {
    var ver = getExternal() && getExternal().getVersion && getExternal().getVersion();
    if (ver) {
      var vd = document.getElementById('versionDisplay');
      if (vd) vd.textContent = 'v' + ver;
    }
  } catch (e) {}

  try {
    var stored = getExternal() && getExternal().readPlayerName && getExternal().readPlayerName();
    if (stored && playerName) playerName.value = stored;
  } catch (e) {}

  try {
    var storedOpts = getExternal() && getExternal().readLaunchOptions && getExternal().readLaunchOptions();
    if (storedOpts) {
      var rawParts = (storedOpts || '').split(' ');
      var parts = [];
      for (var pi = 0; pi < rawParts.length; pi++) {
        var trimmed = (rawParts[pi] || '').toLowerCase().replace(/^\s+|\s+$/g, '');
        if (trimmed) parts.push(trimmed);
      }
      for (var i = 0; i < launchOptionCards.length; i++) {
        var optVal = (launchOptionCards[i].getAttribute('data-option') || '').toLowerCase();
        for (var j = 0; j < parts.length; j++) {
          if (optVal && optVal === parts[j]) {
            launchOptionCards[i].classList.add('active');
            break;
          }
        }
      }
    }
  } catch (e) {}

  try {
    var ex0 = getExternal();
    if (ex0 && ex0.getGamePath) {
      var gp = ex0.getGamePath();
      if (gp) {
        var cpd = document.getElementById('currentPathDisplay');
        if (cpd) cpd.textContent = gp;
        var spd = document.getElementById('settingsPathDisplay');
        if (spd) spd.textContent = gp;
      }
    }
  } catch (e) {}

  window.getPlayerName = function() {
    if (!playerName) return '';
    return (playerName.value || '').replace(/^\s+|\s+$/g, '');
  };

  window.getSelectedLaunchOption = function() {
    var selected = [];
    for (var i = 0; i < launchOptionCards.length; i++) {
      if (launchOptionCards[i].classList.contains('active')) {
        var opt = launchOptionCards[i].getAttribute('data-option') || '';
        if (opt) selected.push(opt);
      }
    }
    return selected.join(' ');
  };

  for (var oi = 0; oi < launchOptionCards.length; oi++) {
    (function(card) {
      card.onclick = function(e) {
        e.stopPropagation();
        card.classList.toggle('active');
      };
    })(launchOptionCards[oi]);
  }

  var navBtns = document.querySelectorAll('.nav-btn');
  var pages = document.querySelectorAll('.page');

  function setPage(targetPage) {
    for (var j = 0; j < navBtns.length; j++) {
      navBtns[j].classList.remove('active');
      if (navBtns[j].getAttribute('data-page') === targetPage) navBtns[j].classList.add('active');
    }
    for (var k = 0; k < pages.length; k++) {
      pages[k].classList.remove('active');
    }
    var targetEl = document.getElementById(targetPage + 'Page');
    if (targetEl) targetEl.classList.add('active');

    if (targetPage === 'workshop' && workshopBrowseItems.length === 0 && !workshopBrowseLoading) {
      try {
        var exP = getExternal();
        if (exP && exP.workshopGetBrowseData) {
          var cachedD = exP.workshopGetBrowseData();
          if (cachedD && cachedD !== '[]') {
            var parsedD = JSON.parse(cachedD);
            if (parsedD && Array.isArray(parsedD) && parsedD.length > 0) {
              workshopBrowseItems = parsedD;
              renderWorkshopBrowse();
            }
          }
        }
      } catch (e) {}
      if (workshopBrowseItems.length === 0) {
        var lcached = loadWorkshopCache();
        if (lcached && lcached.length > 0) {
          workshopBrowseItems = lcached;
          renderWorkshopBrowse();
        }
      }
      loadWorkshopBrowse(1, '');
    }
  }

  for (var ni = 0; ni < navBtns.length; ni++) {
    (function(btn) {
      btn.onclick = function() { setPage(btn.getAttribute('data-page')); };
    })(navBtns[ni]);
  }

  function showMessage(title, body) {
    var pop = document.getElementById('messagePopup');
    if (!pop) return;
    var t = document.getElementById('messagePopupTitle');
    var b = document.getElementById('messagePopupBody');
    if (t) t.textContent = title || 'Message';
    if (b) b.textContent = body || '';
    pop.classList.add('active');
  }

  function showConfirm(title, body, onConfirm) {
    var popup = document.getElementById('confirmPopup');
    if (!popup) return;
    var t = document.getElementById('confirmPopupTitle');
    var b = document.getElementById('confirmPopupBody');
    if (t) t.textContent = title || 'Confirm';
    if (b) b.innerHTML = body || '';
    popup.classList.add('active');
    var done = false;
    function close() {
      if (done) return;
      done = true;
      popup.classList.remove('active');
    }
    document.getElementById('confirmPopupCancel').onclick = function() { close(); };
    document.getElementById('confirmPopupConfirm').onclick = function() {
      close();
      if (typeof onConfirm === 'function') onConfirm();
    };
    var cClose = popup.querySelector('.popup-close');
    if (cClose) cClose.onclick = close;
    popup.onclick = function(e) { if (e.target === popup) close(); };
  }

  function hideMessagePopup() {
    var pop = document.getElementById('messagePopup');
    if (pop) pop.classList.remove('active');
  }

  document.getElementById('messagePopupOk').onclick = hideMessagePopup;
  document.getElementById('messagePopup').onclick = function(e) {
    if (e.target === document.getElementById('messagePopup')) hideMessagePopup();
  };
  var msgClose = document.querySelector('#messagePopup .popup-close');
  if (msgClose) msgClose.onclick = hideMessagePopup;

  function showProgress(message, percent, details) {
    var text = message || '';
    if (details) text += ' \u2014 ' + details;
    progressInfo.textContent = text;
    var pf = document.getElementById('progressFill');
    if (percent < 0) {
      if (pf && !pf.classList.contains('indeterminate')) pf.classList.add('indeterminate');
      progressPercent.textContent = '';
    } else {
      if (pf && pf.classList.contains('indeterminate')) { pf.classList.remove('indeterminate'); pf.style.transform = ''; }
      progressFill.style.width = percent + '%';
      progressPercent.textContent = percent > 0 ? (percent.toFixed(1) + '%') : '0%';
    }
    progressFooter.classList.add('active');
  }

  function hideProgress() {
    progressFooter.classList.remove('active');
  }

  function cancelWorkshopPoll() {
    if (_workshopBrowsePollInterval) {
      clearInterval(_workshopBrowsePollInterval);
      _workshopBrowsePollInterval = null;
    }
    workshopBrowseLoading = false;
  }

  function loadWorkshopBrowse(page, searchTerm) {
    cancelWorkshopPoll();
    workshopBrowseLoading = true;
    workshopBrowseCurrentPage = page || 1;
    workshopBrowseSearchTerm = '';
    if (workshopBrowseItems.length === 0) showWorkshopLoading();
    try {
      var ex = getExternal();
      if (ex && ex.workshopBrowse) {
        ex.workshopBrowse(workshopBrowseCurrentPage);
        pollWorkshopBrowseResults();
      }
    } catch (e) {
      workshopBrowseLoading = false;
      showErrorOrCache('Workshop browsing not available. Showing cached items...');
    }
  }

  function loadWorkshopSearch(query) {
    if (!query || !query.replace(/^\s+|\s+$/g, '')) return;
    cancelWorkshopPoll();
    workshopBrowseLoading = true;
    workshopBrowseSearchTerm = '';
    workshopBrowseCurrentPage = 1;
    showWorkshopLoading();
    try {
      var ex = getExternal();
      if (ex && ex.workshopSearch) {
        ex.workshopSearch(query.replace(/^\s+|\s+$/g, ''));
        pollWorkshopBrowseResults(true);
      } else {
        workshopBrowseLoading = false;
        workshopBrowseSearchTerm = query.replace(/^\s+|\s+$/g, '');
        renderWorkshopBrowse();
      }
    } catch (e) {
      workshopBrowseLoading = false;
      workshopBrowseSearchTerm = query.replace(/^\s+|\s+$/g, '');
      renderWorkshopBrowse();
    }
  }

  function pollWorkshopBrowseResults(isSearchMode) {
    if (_workshopBrowsePollInterval) { clearInterval(_workshopBrowsePollInterval); _workshopBrowsePollInterval = null; }
    var pollCount = 0;
    var maxPolls = 600;
    var hasShownCachedData = workshopBrowseItems.length > 0;
    var pollInterval = setInterval(function() {
      pollCount++;
      _workshopBrowsePollInterval = pollInterval;
      if (!isSearchMode && !hasShownCachedData && pollCount === 20) {
        try {
          var ex3 = getExternal();
          if (ex3 && ex3.workshopGetBrowseData) {
            var cj = ex3.workshopGetBrowseData();
            if (cj && cj !== '[]') {
              var cd = typeof cj === 'string' ? JSON.parse(cj) : cj;
              if (cd && Array.isArray(cd) && cd.length > 0) {
                workshopBrowseItems = cd;
                hasShownCachedData = true;
                workshopBrowseCurrentPage = 1;
                renderWorkshopBrowse();
              }
            }
          }
        } catch (e) {}
      }
      if (pollCount > maxPolls) {
        clearInterval(pollInterval);
        workshopBrowseLoading = false;
        if (workshopBrowseItems.length === 0) showErrorOrCache('Loading took too long. Showing cached items...');
        return;
      }
      try {
        var ex2 = getExternal();
        if (ex2 && ex2.workshopIsBrowseLoading) {
          var isLoading = ex2.workshopIsBrowseLoading() === 'true';
          if (!isLoading) {
            var dataJson = ex2.workshopGetBrowseData ? ex2.workshopGetBrowseData() : null;
            clearInterval(pollInterval);
            if (dataJson) {
              var data = typeof dataJson === 'string' ? JSON.parse(dataJson) : dataJson;
              if (data && Array.isArray(data) && data.length > 0) {
                workshopBrowseItems = data;
                if (!isSearchMode) saveWorkshopCache(workshopBrowseItems);
                workshopBrowseCurrentPage = 1;
                renderWorkshopBrowse();
              } else if (workshopBrowseItems.length === 0) {
                showErrorOrCache('No workshop items found.');
              }
            }
            workshopBrowseLoading = false;
          }
        }
      } catch (e) {
        clearInterval(pollInterval);
        workshopBrowseLoading = false;
        if (workshopBrowseItems.length === 0) showErrorOrCache('Error loading workshop data.');
      }
    }, 100);
    _workshopBrowsePollInterval = pollInterval;
  }

  function showWorkshopLoading() {
    workshopBrowseGrid.innerHTML = '<div class="workshop-loading"><div class="loading-spinner"></div><span>Loading workshop items...</span></div>';
  }

  function showWorkshopEmpty(message) {
    workshopBrowseGrid.innerHTML = '<div class="workshop-browse-empty"><div class="workshop-browse-empty-icon">\u2715</div><div>' + (message || 'No items found') + '</div></div>';
    if (workshopBrowsePagination) workshopBrowsePagination.style.display = 'none';
  }

  function showErrorOrCache(errorMessage) {
    try {
      var ex = getExternal();
      if (ex && ex.workshopGetBrowseData) {
        var fd = ex.workshopGetBrowseData();
        if (fd && fd !== '[]') {
          var parsed = JSON.parse(fd);
          if (parsed && Array.isArray(parsed) && parsed.length > 0) {
            workshopBrowseItems = parsed;
            renderWorkshopBrowse();
            return;
          }
        }
      }
    } catch (e) {}
    var cached = loadWorkshopCache();
    if (cached && cached.length > 0) {
      workshopBrowseItems = cached;
      renderWorkshopBrowse();
    } else {
      showWorkshopEmpty(errorMessage);
    }
  }

  function renderWorkshopBrowse() {
    workshopBrowseGrid.innerHTML = '';
    var nonArchive = [];
    for (var ai = 0; ai < workshopBrowseItems.length; ai++) {
      var item = workshopBrowseItems[ai];
      if (item.title && (item.title.indexOf('**ARCHIVE**') !== -1 || item.title.indexOf('ARCHIVE') === 0)) continue;
      if (item.description && item.description.indexOf('THIS MAP IS AN ARCHIVE') !== -1) continue;
      nonArchive.push(item);
    }

    var filteredItems = nonArchive;
    if (workshopBrowseSearchTerm.length > 0) {
      var term = workshopBrowseSearchTerm.toLowerCase().replace(/^\s+|\s+$/g, '');
      var searched = [];
      for (var si = 0; si < nonArchive.length; si++) {
        var it = nonArchive[si];
        var titleMatch = it.title && it.title.toLowerCase().indexOf(term) !== -1;
        var descMatch = it.description && it.description.toLowerCase().indexOf(term) !== -1;
        var idMatch = it.id && String(it.id).toLowerCase().indexOf(term) !== -1;
        if (titleMatch || descMatch || idMatch) searched.push(it);
      }
      filteredItems = searched;
    }

    var sortMode = workshopFilterSelect ? workshopFilterSelect.value : 'mostrecent';
    if (sortMode === 'trend') {
      filteredItems = filteredItems.slice(0).sort(function(a, b) {
        var rA = parseInt(a.starRating, 10) || 0;
        var rB = parseInt(b.starRating, 10) || 0;
        if (rB !== rA) return rB - rA;
        return (parseInt(b.subs, 10) || 0) - (parseInt(a.subs, 10) || 0);
      });
    } else if (sortMode === 'oldest') {
      filteredItems = filteredItems.slice(0).reverse();
    } else if (sortMode === 'alphabetical') {
      filteredItems = filteredItems.slice(0).sort(function(a, b) {
        var ta = (a.title || '').toLowerCase();
        var tb = (b.title || '').toLowerCase();
        return ta < tb ? -1 : (ta > tb ? 1 : 0);
      });
    }

    if (!filteredItems || filteredItems.length === 0) {
      showWorkshopEmpty(workshopBrowseSearchTerm ? 'No items match your search' : 'No workshop items available');
      return;
    }

    var totalPages = Math.ceil(filteredItems.length / workshopBrowseItemsPerPage) || 1;
    if (workshopBrowseCurrentPage < 1) workshopBrowseCurrentPage = 1;
    if (workshopBrowseCurrentPage > totalPages) workshopBrowseCurrentPage = totalPages;

    var start = (workshopBrowseCurrentPage - 1) * workshopBrowseItemsPerPage;
    var end = Math.min(start + workshopBrowseItemsPerPage, filteredItems.length);

    for (var i = start; i < end; i++) {
      var it = filteredItems[i];
      var card = document.createElement('div');
      card.className = 'workshop-item-card';

      var imageHtml = it.imageUrl
        ? '<div class="workshop-item-image"><img src="' + escapeHtml(it.imageUrl) + '" alt=""></div>'
        : '<div class="workshop-item-image"><div class="no-image">No Image</div></div>';

      var rawDesc = stripBBCode((it.description || '').replace(/[\r\n]+/g, ' '));
      var desc = rawDesc.substring(0, 90);
      if (rawDesc.length > 90) desc += '...';

      var ratingHtml = '';
      var sr = parseInt(it.starRating, 10) || 0;
      if (sr > 0) {
        var starStr = '';
        for (var s = 0; s < sr; s++) starStr += '\u2605';
        for (var s = sr; s < 5; s++) starStr += '\u2606';
        var rColor = sr >= 4 ? 'rgba(34,197,94,0.8)' : (sr >= 3 ? 'rgba(250,204,21,0.8)' : 'rgba(239,68,68,0.8)');
        ratingHtml = '<span style="font-size:0.72rem;color:' + rColor + ';margin-left:7px;">' + starStr + '</span>';
      }

      var fSize = parseInt(it.file_size, 10) || 0;
      var fStr = '';
      if (fSize > 0) {
        fStr = fSize > 1073741824 ? (fSize / 1073741824).toFixed(2) + ' GB'
          : fSize > 1048576 ? (fSize / 1048576).toFixed(1) + ' MB'
          : fSize > 1024 ? (fSize / 1024).toFixed(0) + ' KB' : fSize + ' B';
      }

      var contentHtml = '<div class="workshop-item-content">'
        + '<div style="flex:1;min-width:0;">'
        + '<div class="workshop-item-title">' + escapeHtml(it.title || 'Untitled') + ratingHtml + '</div>'
        + (desc ? '<div style="font-size:0.75rem;color:rgba(200,196,192,0.55);margin-top:2px;line-height:1.3;">' + escapeHtml(desc) + '</div>' : '')
        + '<div style="font-size:0.7rem;color:rgba(200,196,192,0.35);margin-top:3px;font-family:Consolas,monospace;">ID: ' + escapeHtml(it.id) + '</div>'
        + '</div>'
        + '<div style="display:flex;flex-direction:column;align-items:center;flex-shrink:0;margin-left:10px;gap:3px;">'
        + '<button type="button" class="workshop-item-button" data-workshop-id="' + escapeHtml(it.id) + '">Download</button>'
        + (fStr ? '<span style="font-size:0.65rem;color:rgba(200,196,192,0.35);">' + fStr + '</span>' : '')
        + '</div>'
        + '</div>';

      card.innerHTML = imageHtml + contentHtml;

      var cardImg = card.querySelector('img');
      if (cardImg) {
        (function(img) {
          img.onerror = function() {
            this.onerror = null;
            this.parentNode.innerHTML = '<div class="no-image">No Image</div>';
          };
        })(cardImg);
      }

      workshopBrowseGrid.appendChild(card);
    }

    if (workshopBrowsePagination) {
      if (totalPages > 1) {
        workshopBrowsePagination.style.display = 'flex';
        workshopBrowsePagination.innerHTML = '';

        var prevBtn = document.createElement('button');
        prevBtn.type = 'button';
        prevBtn.className = 'btn';
        prevBtn.textContent = '\u2039 Prev';
        prevBtn.disabled = workshopBrowseCurrentPage <= 1;
        prevBtn.onclick = function() {
          if (workshopBrowseCurrentPage > 1) {
            workshopBrowseCurrentPage--;
            renderWorkshopBrowse();
            workshopBrowseGrid.scrollTop = 0;
          }
        };
        workshopBrowsePagination.appendChild(prevBtn);

        var pages2 = [];
        if (totalPages <= 7) {
          for (var p = 1; p <= totalPages; p++) pages2.push(p);
        } else {
          pages2.push(1);
          if (workshopBrowseCurrentPage > 4) pages2.push('...');
          var rangeStart = Math.max(2, workshopBrowseCurrentPage - 2);
          var rangeEnd = Math.min(totalPages - 1, workshopBrowseCurrentPage + 2);
          if (workshopBrowseCurrentPage <= 4) rangeEnd = Math.min(totalPages - 1, 5);
          if (workshopBrowseCurrentPage >= totalPages - 3) rangeStart = Math.max(2, totalPages - 4);
          for (var p = rangeStart; p <= rangeEnd; p++) pages2.push(p);
          if (workshopBrowseCurrentPage < totalPages - 3) pages2.push('...');
          pages2.push(totalPages);
        }

        for (var pii = 0; pii < pages2.length; pii++) {
          if (pages2[pii] === '...') {
            var dots = document.createElement('span');
            dots.className = 'page-ellipsis';
            dots.textContent = '...';
            workshopBrowsePagination.appendChild(dots);
          } else {
            var pageBtn = document.createElement('button');
            pageBtn.type = 'button';
            pageBtn.className = 'btn' + (pages2[pii] === workshopBrowseCurrentPage ? ' page-active' : '');
            pageBtn.textContent = String(pages2[pii]);
            pageBtn.setAttribute('data-page', pages2[pii]);
            pageBtn.onclick = function() {
              var pg = parseInt(this.getAttribute('data-page'), 10);
              if (pg !== workshopBrowseCurrentPage) {
                workshopBrowseCurrentPage = pg;
                renderWorkshopBrowse();
                workshopBrowseGrid.scrollTop = 0;
              }
            };
            workshopBrowsePagination.appendChild(pageBtn);
          }
        }

        var nextBtn = document.createElement('button');
        nextBtn.type = 'button';
        nextBtn.className = 'btn';
        nextBtn.textContent = 'Next \u203A';
        nextBtn.disabled = workshopBrowseCurrentPage >= totalPages;
        nextBtn.onclick = function() {
          if (workshopBrowseCurrentPage < totalPages) {
            workshopBrowseCurrentPage++;
            renderWorkshopBrowse();
            workshopBrowseGrid.scrollTop = 0;
          }
        };
        workshopBrowsePagination.appendChild(nextBtn);

        var info = document.createElement('span');
        info.className = 'page-info';
        info.textContent = filteredItems.length + ' items';
        workshopBrowsePagination.appendChild(info);
      } else {
        workshopBrowsePagination.style.display = 'none';
      }
    }
  }

  function showWorkshopModal(item) {
    if (!item) return;
    workshopModalTitle.textContent = item.title || 'Untitled';
    if (item.imageUrl) {
      workshopModalImage.innerHTML = '<img src="' + escapeHtml(item.imageUrl) + '" alt="">';
      var mImg = workshopModalImage.querySelector('img');
      if (mImg) {
        mImg.onerror = function() {
          this.onerror = null;
          workshopModalImage.innerHTML = '<div style="display:flex;align-items:center;justify-content:center;height:100%;color:rgba(200,196,192,0.25);font-size:2rem;">No Image</div>';
        };
      }
    } else {
      workshopModalImage.innerHTML = '<div style="display:flex;align-items:center;justify-content:center;height:100%;color:rgba(200,196,192,0.25);font-size:2rem;">No Image</div>';
    }

    var modalRatingHtml = '<span style="color:rgba(200,196,192,0.5);">N/A</span>';
    var mSr = parseInt(item.starRating, 10) || 0;
    if (mSr > 0) {
      var mStars = '';
      for (var ms = 0; ms < mSr; ms++) mStars += '\u2605';
      for (var ms = mSr; ms < 5; ms++) mStars += '\u2606';
      var mColor = mSr >= 4 ? 'rgba(34,197,94,0.9)' : (mSr >= 3 ? 'rgba(250,204,21,0.9)' : 'rgba(239,68,68,0.9)');
      modalRatingHtml = '<span style="color:' + mColor + ';">' + mStars + ' (' + mSr + '/5)</span>';
    } else {
      modalRatingHtml = '<span style="color:rgba(200,196,192,0.45);">Not enough ratings</span>';
    }

    var modalSubsHtml = '';
    var mSubs = parseInt(item.subs, 10) || 0;
    var mFavs = parseInt(item.favorites, 10) || 0;
    if (mSubs > 0 || mFavs > 0) {
      modalSubsHtml = '<div class="workshop-modal-info-item">'
        + '<div class="workshop-modal-info-label">Popularity</div>'
        + '<div class="workshop-modal-info-value">'
        + (mSubs > 0 ? mSubs.toLocaleString() + ' subs' : '')
        + (mSubs > 0 && mFavs > 0 ? ' \u00B7 ' : '')
        + (mFavs > 0 ? mFavs.toLocaleString() + ' favs' : '')
        + '</div></div>';
    }

    var modalSizeHtml = '';
    var mSize = parseInt(item.file_size, 10) || 0;
    if (mSize > 0) {
      var sStr = mSize > 1073741824 ? (mSize / 1073741824).toFixed(2) + ' GB'
        : mSize > 1048576 ? (mSize / 1048576).toFixed(1) + ' MB'
        : mSize > 1024 ? (mSize / 1024).toFixed(0) + ' KB' : mSize + ' B';
      modalSizeHtml = '<div class="workshop-modal-info-item">'
        + '<div class="workshop-modal-info-label">Size</div>'
        + '<div class="workshop-modal-info-value">' + sStr + '</div></div>';
    }

    workshopModalInfo.innerHTML =
      '<div class="workshop-modal-info-item">'
      + '<div class="workshop-modal-info-label">Workshop ID</div>'
      + '<div class="workshop-modal-info-value" style="font-family:Consolas,monospace;">' + escapeHtml(item.id || 'N/A') + '</div>'
      + '</div>'
      + '<div class="workshop-modal-info-item">'
      + '<div class="workshop-modal-info-label">Rating</div>'
      + '<div class="workshop-modal-info-value">' + modalRatingHtml + '</div>'
      + '</div>'
      + modalSubsHtml + modalSizeHtml;

    workshopModalDescription.innerHTML = convertBBCodeToHtml(item.description || 'No description available.');
    workshopModalViewBtn.onclick = function() {
      try { window.external.openUrl('https://steamcommunity.com/sharedfiles/filedetails/?id=' + item.id); } catch (e) {}
    };
    workshopModalDownloadBtn.onclick = function() {
      hideWorkshopModal();
      startWorkshopDownload(item.id, item.title);
    };
    workshopOverlay.classList.add('active');
    workshopModal.classList.add('active');
  }

  function hideWorkshopModal() {
    workshopOverlay.classList.remove('active');
    workshopModal.classList.remove('active');
  }

  if (workshopModalClose) workshopModalClose.onclick = hideWorkshopModal;
  if (workshopOverlay) workshopOverlay.onclick = hideWorkshopModal;
  if (workshopModal) workshopModal.onclick = function(e) { e.stopPropagation(); };

  if (workshopFilterSelect) {
    workshopFilterSelect.onchange = function() {
      workshopBrowseCurrentPage = 1;
      renderWorkshopBrowse();
    };
  }

  if (workshopSearchBtn) {
    workshopSearchBtn.onclick = function() {
      var st = workshopSearchInput ? workshopSearchInput.value.replace(/^\s+|\s+$/g, '') : '';
      if (!st) {
        workshopBrowseSearchTerm = '';
        workshopBrowseCurrentPage = 1;
        if (workshopBrowseItems.length > 0) renderWorkshopBrowse();
        else loadWorkshopBrowse(1);
        return;
      }
      loadWorkshopSearch(st);
    };
  }

  if (workshopSearchInput) {
    workshopSearchInput.onkeypress = function(e) {
      if (e.keyCode === 13 || e.key === 'Enter') workshopSearchBtn.click();
    };
  }

  if (workshopBrowseRefreshBtn) {
    workshopBrowseRefreshBtn.onclick = function() {
      workshopBrowseSearchTerm = '';
      if (workshopSearchInput) workshopSearchInput.value = '';
      workshopBrowseCurrentPage = 1;
      loadWorkshopBrowse(1);
    };
  }

  if (workshopBrowseClearBtn) {
    workshopBrowseClearBtn.onclick = function() {
      workshopBrowseItems = [];
      workshopBrowseSearchTerm = '';
      workshopBrowseCurrentPage = 1;
      if (workshopSearchInput) workshopSearchInput.value = '';
      try { localStorage.removeItem(workshopBrowseCacheKey); } catch (e) {}
      loadWorkshopBrowse(1);
    };
  }

  workshopBrowseGrid.addEventListener('click', function(e) {
    var btn = e.target.closest ? e.target.closest('.workshop-item-button') : null;
    if (!btn) {
      var t = e.target;
      while (t && t !== workshopBrowseGrid) {
        if (t.className && t.className.indexOf('workshop-item-button') !== -1) { btn = t; break; }
        t = t.parentNode;
      }
    }
    if (btn) {
      var id = btn.getAttribute('data-workshop-id');
      if (id) {
        var foundItem = null;
        for (var fi = 0; fi < workshopBrowseItems.length; fi++) {
          if (workshopBrowseItems[fi].id === id) { foundItem = workshopBrowseItems[fi]; break; }
        }
        startWorkshopDownload(id, foundItem ? foundItem.title : id);
      }
      e.preventDefault();
      e.stopPropagation();
      return;
    }
    var card = e.target.closest ? e.target.closest('.workshop-item-card') : null;
    if (!card) {
      var t2 = e.target;
      while (t2 && t2 !== workshopBrowseGrid) {
        if (t2.className && t2.className.indexOf('workshop-item-card') !== -1) { card = t2; break; }
        t2 = t2.parentNode;
      }
    }
    if (card) {
      var cBtn = card.querySelector('.workshop-item-button');
      var cid = cBtn ? cBtn.getAttribute('data-workshop-id') : null;
      if (cid) {
        var found = null;
        for (var fi2 = 0; fi2 < workshopBrowseItems.length; fi2++) {
          if (workshopBrowseItems[fi2].id === cid) { found = workshopBrowseItems[fi2]; break; }
        }
        if (found) showWorkshopModal(found);
      }
      e.preventDefault();
      e.stopPropagation();
    }
  });

  function stopWorkshopDownloadCompletely() {
    try {
      var ex = getExternal();
      if (ex && ex.workshopCancelDownload) ex.workshopCancelDownload();
    } catch (e) {}
    if (window._workshopPollInterval) { clearInterval(window._workshopPollInterval); window._workshopPollInterval = null; }
    window._workshopIsPaused = false;
    var pauseBtn = document.getElementById('workshopProgressPause');
    var pauseFooter = document.getElementById('progressPause');
    if (pauseBtn) { pauseBtn.innerHTML = '&#9208;'; pauseBtn.title = 'Pause'; }
    if (pauseFooter) { pauseFooter.innerHTML = '&#9208;'; pauseFooter.title = 'Pause'; }
    workshopDownloadBtn.disabled = false;
    workshopProgress.style.display = 'none';
    var msgEl = document.getElementById('workshopStatusMessage');
    if (msgEl) { msgEl.textContent = ''; msgEl.style.color = 'rgba(249,115,22,0.9)'; }
    var detEl = document.getElementById('workshopStatusDetails');
    if (detEl) detEl.textContent = '';
    var pctEl = document.getElementById('workshopProgressPercent');
    if (pctEl) pctEl.textContent = '';
    workshopProgressFill.style.width = '0%';
    workshopProgressFill.style.transform = '';
    workshopProgressFill.classList.remove('indeterminate');
    window._wsIndeterminate = false;
    hideProgress();
    setTimeout(refreshModsGrid, 300);
  }

  function doStartWorkshopDownload(id, displayName) {
    try {
      var ex = getExternal();
      if (ex && ex.workshopDownload) {
        if (ex.workshopCheckInstalled) {
          var inst = ex.workshopCheckInstalled(String(id));
          if (inst && inst.length > 0) {
            showMessage('Already Installed', 'This workshop item is already installed at:\n' + inst + '\n\nRemove it first if you want to reinstall.');
            return;
          }
        }
        ex.workshopDownload(String(id));
        var msgEl = document.getElementById('workshopStatusMessage');
        var detEl = document.getElementById('workshopStatusDetails');
        var pctEl = document.getElementById('workshopProgressPercent');
        if (msgEl) { msgEl.textContent = 'Initializing...'; msgEl.style.color = 'rgba(249,115,22,0.9)'; }
        if (detEl) detEl.textContent = 'Workshop ID: ' + id;
        if (pctEl) pctEl.textContent = '';
        window._lastWorkshopTerminalState = '';
        window._wsMaxProgress = 0;
        window._wsIndeterminate = true;
        workshopProgress.style.display = 'block';
        workshopProgressFill.style.width = '0%';
        workshopProgressFill.classList.add('indeterminate');
        workshopDownloadBtn.disabled = true;
        showProgress('Starting download for ' + (displayName || id) + '...', -1);
        if (window._workshopPollInterval) { clearInterval(window._workshopPollInterval); window._workshopPollInterval = null; }
        window._workshopPollInterval = setInterval(function() {
          pollWorkshopStatus();
          try {
            var ex2 = getExternal();
            if (ex2 && ex2.workshopGetStatus) {
              var sj = ex2.workshopGetStatus();
              if (sj) {
                var st = typeof sj === 'string' ? JSON.parse(sj) : sj;
                if (st.message && (st.message.indexOf('Done') !== -1 || st.message.indexOf('Error') !== -1 || st.message.indexOf('Canceled') !== -1 || st.message.indexOf('Already installed') !== -1)) {
                  clearInterval(window._workshopPollInterval);
                  window._workshopPollInterval = null;
                  workshopDownloadBtn.disabled = false;
                }
              }
            }
          } catch (e) {
            clearInterval(window._workshopPollInterval);
            window._workshopPollInterval = null;
            workshopDownloadBtn.disabled = false;
          }
        }, 500);
      }
    } catch (e) {}
  }

  function startWorkshopDownload(id, displayName) {
    if (window._workshopPollInterval) {
      showConfirm('Download in progress', 'A download is already in progress. Stop it and download "' + (displayName || id) + '" instead?', function() {
        stopWorkshopDownloadCompletely();
        setTimeout(function() { doStartWorkshopDownload(id, displayName); }, 300);
      });
      return;
    }
    doStartWorkshopDownload(id, displayName);
  }

  function pollWorkshopStatus() {
    try {
      var ex = getExternal();
      if (!ex || !ex.workshopGetStatus) return;
      var statusJson = ex.workshopGetStatus();
      if (!statusJson || statusJson.length === 0) return;
      var status = typeof statusJson === 'string' ? JSON.parse(statusJson) : statusJson;
      if (status.downloadFolder) _workshopDownloadFolder = status.downloadFolder;
      if (!window._lastWorkshopTerminalState) window._lastWorkshopTerminalState = '';

      if (status.paused !== undefined) {
        var pauseBtn = document.getElementById('workshopProgressPause');
        var pauseFooter = document.getElementById('progressPause');
        if (status.paused && !window._workshopIsPaused) {
          window._workshopIsPaused = true;
          if (pauseBtn) { pauseBtn.innerHTML = '&#9654;'; pauseBtn.title = 'Resume'; }
          if (pauseFooter) { pauseFooter.innerHTML = '&#9654;'; pauseFooter.title = 'Resume'; }
        } else if (!status.paused && window._workshopIsPaused) {
          window._workshopIsPaused = false;
          if (pauseBtn) { pauseBtn.innerHTML = '&#9208;'; pauseBtn.title = 'Pause'; }
          if (pauseFooter) { pauseFooter.innerHTML = '&#9208;'; pauseFooter.title = 'Pause'; }
        }
      }

      var msgEl = document.getElementById('workshopStatusMessage');
      var detEl = document.getElementById('workshopStatusDetails');
      var pctEl = document.getElementById('workshopProgressPercent');

      if (status.message && status.message.length > 0) {
        workshopProgress.style.display = 'block';
        if (msgEl) msgEl.textContent = status.message;
        if (detEl) detEl.textContent = status.details || '';

        if (status.progress !== undefined && status.progress >= 0) {
          if (window._wsIndeterminate) {
            window._wsIndeterminate = false;
            workshopProgressFill.classList.remove('indeterminate');
            workshopProgressFill.style.transform = '';
            var pf2 = document.getElementById('progressFill');
            if (pf2) { pf2.classList.remove('indeterminate'); pf2.style.transform = ''; }
          }
          var p = Math.min(status.progress, 99);
          workshopProgressFill.style.width = p + '%';
          if (pctEl) pctEl.textContent = p > 0 ? (p.toFixed(1) + '%') : '';
          showProgress(status.message, p, status.details);
        } else if (status.progress !== undefined && status.progress < 0) {
          if (!window._wsIndeterminate) {
            window._wsIndeterminate = true;
            workshopProgressFill.classList.add('indeterminate');
            var pf3 = document.getElementById('progressFill');
            if (pf3) pf3.classList.add('indeterminate');
          }
          if (pctEl) pctEl.textContent = '';
          showProgress(status.message, -1, status.details);
        } else {
          workshopProgressFill.style.width = '0%';
          if (pctEl) pctEl.textContent = '';
          showProgress(status.message, 0, status.details);
        }

        var isDone = status.message.indexOf('Done') !== -1;
        var isError = status.message.indexOf('Error') !== -1;
        var isCanceled = status.message.indexOf('Canceled') !== -1;
        var isAlready = status.message.indexOf('Already installed') !== -1;

        if (isDone || isError || isCanceled || isAlready) {
          if (window._workshopPollInterval) { clearInterval(window._workshopPollInterval); window._workshopPollInterval = null; }
          workshopDownloadBtn.disabled = false;
          setTimeout(refreshModsGrid, 500);
          if (window._lastWorkshopTerminalState !== status.message) {
            window._lastWorkshopTerminalState = status.message;
            if (isDone) {
              if (msgEl) msgEl.style.color = 'rgba(34,197,94,0.9)';
              showMessage('Workshop', status.message + (status.details ? '\n' + status.details : ''));
            } else if (isCanceled) {
              if (msgEl) msgEl.style.color = 'rgba(250,204,21,0.9)';
            } else if (isAlready) {
              if (msgEl) msgEl.style.color = 'rgba(249,115,22,0.9)';
              showMessage('Already Installed', status.details || 'This item is already installed. Remove it first to reinstall.');
            } else {
              if (msgEl) msgEl.style.color = 'rgba(239,68,68,0.9)';
              showMessage('Workshop Error', status.message + (status.details ? '\n' + status.details : ''));
            }
          }
          if (isCanceled) {
            workshopProgress.style.display = 'none';
            if (msgEl) msgEl.style.color = 'rgba(249,115,22,0.9)';
            hideProgress();
          } else if (isAlready) {
            workshopProgress.style.display = 'none';
            if (msgEl) msgEl.style.color = 'rgba(249,115,22,0.9)';
            hideProgress();
          } else if (isDone) {
            if (workshopId) workshopId.value = '';
            setTimeout(function() {
              workshopProgress.style.display = 'none';
              if (msgEl) msgEl.style.color = 'rgba(249,115,22,0.9)';
            }, 4000);
            setTimeout(hideProgress, 4000);
          } else if (isError) {
            setTimeout(function() {
              workshopProgress.style.display = 'none';
              if (msgEl) msgEl.style.color = 'rgba(249,115,22,0.9)';
            }, 5000);
            setTimeout(hideProgress, 5000);
          }
        }
      }
    } catch (e) {}
  }

  workshopDownloadBtn.onclick = function() {
    var id = (workshopId && workshopId.value || '').replace(/^\s+|\s+$/g, '');
    if (!id) {
      if (workshopStatus) workshopStatus.textContent = 'Enter a Workshop ID or link.';
      return;
    }
    startWorkshopDownload(id, id);
  };

  var workshopProgressCancel = document.getElementById('workshopProgressCancel');
  if (workshopProgressCancel) workshopProgressCancel.onclick = function() { stopWorkshopDownloadCompletely(); };

  var workshopProgressPause = document.getElementById('workshopProgressPause');
  window._workshopIsPaused = false;
  if (workshopProgressPause) {
    workshopProgressPause.onclick = function() {
      try {
        var ex = getExternal();
        if (!ex) return;
        if (window._workshopIsPaused) {
          if (ex.workshopResumeDownload) ex.workshopResumeDownload();
          window._workshopIsPaused = false;
          workshopProgressPause.innerHTML = '&#9208;';
          workshopProgressPause.title = 'Pause';
          var pf = document.getElementById('progressPause');
          if (pf) { pf.innerHTML = '&#9208;'; pf.title = 'Pause'; }
        } else {
          if (ex.workshopPauseDownload) ex.workshopPauseDownload();
          window._workshopIsPaused = true;
          workshopProgressPause.innerHTML = '&#9654;';
          workshopProgressPause.title = 'Resume';
          var pf2 = document.getElementById('progressPause');
          if (pf2) { pf2.innerHTML = '&#9654;'; pf2.title = 'Resume'; }
        }
      } catch (e) {}
    };
  }

  var progressPauseBtn = document.getElementById('progressPause');
  if (progressPauseBtn) {
    progressPauseBtn.onclick = function() {
      if (workshopProgressPause) workshopProgressPause.click();
    };
  }

  document.getElementById('progressCancel').onclick = function() { stopWorkshopDownloadCompletely(); };

  progressOpenFolder.onclick = function() {
    if (_workshopDownloadFolder && _workshopDownloadFolder.length > 0) {
      try { window.external.openUrl(_workshopDownloadFolder); } catch (e) {}
    }
  };

  var _modsListPollInterval = null;
  var _removeProgressPollInterval = null;

  function startRemoveProgressPoll() {
    var bar = document.getElementById('removeProgressBar');
    var msg = document.getElementById('removeMsg');
    var det = document.getElementById('removeDetails');
    var fill = document.getElementById('removeFill');
    if (!bar) return;
    bar.style.display = 'block';
    if (msg) msg.textContent = 'Removing...';
    if (det) det.textContent = '';
    if (fill) { fill.style.width = '0%'; fill.classList.add('indeterminate'); }
    if (_removeProgressPollInterval) clearInterval(_removeProgressPollInterval);
    var pollCount = 0;
    _removeProgressPollInterval = setInterval(function() {
      pollCount++;
      if (pollCount > 600) {
        clearInterval(_removeProgressPollInterval);
        _removeProgressPollInterval = null;
        bar.style.display = 'none';
        refreshModsGrid();
        return;
      }
      try {
        var ex = getExternal();
        if (!ex || !ex.getRemoveStatus) return;
        var sj = ex.getRemoveStatus();
        if (!sj) return;
        var st = typeof sj === 'string' ? JSON.parse(sj) : sj;
        if (msg) msg.textContent = st.message || 'Removing...';
        if (det) det.textContent = st.details || '';
        if (fill) {
          if (st.progress >= 0 && st.progress <= 100) {
            fill.classList.remove('indeterminate');
            fill.style.transform = '';
            fill.style.width = Math.min(st.progress, 100) + '%';
          } else {
            fill.classList.add('indeterminate');
            fill.style.width = '';
          }
        }
        if (!st.running) {
          clearInterval(_removeProgressPollInterval);
          _removeProgressPollInterval = null;
          setTimeout(function() {
            bar.style.display = 'none';
            if (fill) { fill.classList.remove('indeterminate'); fill.style.width = '0%'; fill.style.transform = ''; }
            refreshModsGrid();
          }, 600);
        }
      } catch (e) {}
    }, 200);
  }

  function refreshModsGrid() {
    try {
      var ex = getExternal();
      if (!ex) {
        modsItemsCache = [];
        modsGrid.innerHTML = '<div class="empty-state"><div class="empty-state-icon">&#128230;</div><div class="empty-state-text">No workshop items installed yet</div></div>';
        if (modsPagination) modsPagination.style.display = 'none';
        return;
      }
      if (ex.workshopList) {
        var cached = ex.workshopList();
        if (cached && cached !== '[]') {
          var ci = typeof cached === 'string' ? JSON.parse(cached) : cached;
          if (ci && ci.length > 0) { modsItemsCache = ci; modsPage = 1; renderModsPage(); }
          else { modsItemsCache = []; renderModsPage(); }
        } else { modsItemsCache = []; renderModsPage(); }
      }
      if (ex.workshopListAsync) {
        ex.workshopListAsync();
        if (_modsListPollInterval) clearInterval(_modsListPollInterval);
        var pc = 0;
        _modsListPollInterval = setInterval(function() {
          pc++;
          if (pc > 300) { clearInterval(_modsListPollInterval); _modsListPollInterval = null; return; }
          try {
            var ex2 = getExternal();
            if (!ex2) return;
            var isLoading = ex2.workshopListIsLoading && ex2.workshopListIsLoading() === 'true';
            if (!isLoading) {
              clearInterval(_modsListPollInterval);
              _modsListPollInterval = null;
              var list = ex2.workshopList();
              if (list && list !== '[]') {
                var items = typeof list === 'string' ? JSON.parse(list) : list;
                if (items && items.length > 0) { modsItemsCache = items; modsPage = 1; renderModsPage(); }
                else { modsItemsCache = []; renderModsPage(); }
              } else { modsItemsCache = []; renderModsPage(); }
            }
          } catch (e) {}
        }, 100);
      }
    } catch (e) {
      modsItemsCache = [];
      modsGrid.innerHTML = '<div class="empty-state"><div class="empty-state-icon">&#9888;</div><div class="empty-state-text">Error loading workshop items</div></div>';
      if (modsPagination) modsPagination.style.display = 'none';
    }
  }

  function renderModsPage() {
    modsGrid.innerHTML = '';
    if (!modsItemsCache || modsItemsCache.length === 0) {
      modsGrid.innerHTML = '<div class="empty-state"><div class="empty-state-icon">&#128230;</div><div class="empty-state-text">No workshop items installed yet</div></div>';
      if (modsPagination) modsPagination.style.display = 'none';
      return;
    }
    var totalPages = Math.ceil(modsItemsCache.length / modsPageSize) || 1;
    if (modsPage < 1) modsPage = 1;
    if (modsPage > totalPages) modsPage = totalPages;
    var start = (modsPage - 1) * modsPageSize;
    var end = Math.min(start + modsPageSize, modsItemsCache.length);
    for (var i = start; i < end; i++) {
      var item = modsItemsCache[i];
      var card = document.createElement('div');
      card.className = 'mod-card';

      var header = document.createElement('div');
      header.className = 'mod-card-header';

      var nameDiv = document.createElement('div');
      nameDiv.className = 'mod-card-name';
      nameDiv.textContent = item.name || item.folder;
      nameDiv.title = item.name || item.folder;

      var typeDiv = document.createElement('div');
      typeDiv.className = 'mod-card-type';
      var typeText = (item.type || 'mod') + (item.source === 'steam' ? ' (Steam)' : '');
      if (item.needsUpdate) {
        typeDiv.innerHTML = typeText + ' <span style="color:#ff7a2f;font-weight:bold;margin-left:4px;">&#x21bb; Update</span>';
      } else {
        typeDiv.textContent = typeText;
      }
      header.appendChild(nameDiv);
      header.appendChild(typeDiv);

      var descDiv = null;
      var descText = item.description || '';
      if (descText.length > 0) {
        descDiv = document.createElement('div');
        descDiv.className = 'mod-card-description';
        descDiv.textContent = descText;
      }

      var metaDiv = document.createElement('div');
      metaDiv.className = 'mod-card-meta';
      var hasMeta = false;

      var lSz = parseInt(item.localSize, 10) || 0;
      var aSz = parseInt(item.file_size, 10) || 0;
      var sz = lSz > 0 ? lSz : aSz;
      if (sz > 0) {
        var szStr = sz > 1073741824 ? (sz / 1073741824).toFixed(2) + ' GB'
          : sz > 1048576 ? (sz / 1048576).toFixed(1) + ' MB'
          : sz > 1024 ? (sz / 1024).toFixed(0) + ' KB' : sz + ' B';
        var sizeSpan = document.createElement('span');
        sizeSpan.innerHTML = '<span class="meta-icon">&#128190;</span> ' + szStr;
        metaDiv.appendChild(sizeSpan);
        hasMeta = true;
      }

      var sr = parseInt(item.starRating, 10) || 0;
      if (sr > 0) {
        var sStr2 = '';
        for (var s = 0; s < sr; s++) sStr2 += '\u2605';
        for (var s = sr; s < 5; s++) sStr2 += '\u2606';
        var rColor2 = sr >= 4 ? 'rgba(34,197,94,0.75)' : (sr >= 3 ? 'rgba(250,204,21,0.75)' : 'rgba(239,68,68,0.75)');
        var ratingSpan = document.createElement('span');
        ratingSpan.style.color = rColor2;
        ratingSpan.textContent = sStr2;
        metaDiv.appendChild(ratingSpan);
        hasMeta = true;
      }

      var mSubs = parseInt(item.subs, 10) || 0;
      if (mSubs > 0) {
        var subsSpan = document.createElement('span');
        subsSpan.innerHTML = '<span class="meta-icon">&#128101;</span> ' + mSubs.toLocaleString();
        metaDiv.appendChild(subsSpan);
        hasMeta = true;
      }

      var actions = document.createElement('div');
      actions.className = 'mod-card-actions';

      if (item.id && String(item.id).length > 0) {
        var viewBtn = document.createElement('button');
        viewBtn.className = 'btn';
        viewBtn.type = 'button';
        viewBtn.textContent = 'View';
        viewBtn.setAttribute('data-open-url', 'https://steamcommunity.com/sharedfiles/filedetails/?id=' + String(item.id));
        actions.appendChild(viewBtn);

        var updateBtn = document.createElement('button');
        updateBtn.className = 'btn';
        if (item.needsUpdate) {
          updateBtn.style.background = 'linear-gradient(145deg,rgba(249,115,22,0.45),rgba(220,90,12,0.35))';
          updateBtn.style.borderColor = 'rgba(249,115,22,0.7)';
          updateBtn.style.fontWeight = 'bold';
        }
        updateBtn.type = 'button';
        updateBtn.textContent = item.needsUpdate ? 'Update Now' : 'Update';
        (function(itemId, itemName) {
          updateBtn.onclick = function(e) {
            e.stopPropagation();
            try {
              var ex = getExternal();
              if (ex && ex.workshopDownload) {
                ex.workshopDownload(String(itemId));
                workshopProgress.style.display = 'block';
                workshopProgressFill.style.width = '0%';
                workshopDownloadBtn.disabled = true;
                var pi2 = setInterval(function() {
                  pollWorkshopStatus();
                  var ex2 = getExternal();
                  if (ex2 && ex2.workshopGetStatus) {
                    var sj = ex2.workshopGetStatus();
                    if (sj) {
                      var st = typeof sj === 'string' ? JSON.parse(sj) : sj;
                      if (st.message && (st.message.indexOf('Done') !== -1 || st.message.indexOf('Error') !== -1 || st.message.indexOf('Already installed') !== -1)) {
                        clearInterval(pi2);
                        workshopDownloadBtn.disabled = false;
                        setTimeout(refreshModsGrid, 500);
                      }
                    }
                  }
                }, 500);
              }
            } catch (err) {}
          };
        })(item.id, item.name || item.folder);
        actions.appendChild(updateBtn);
      }

      var removeBtn = document.createElement('button');
      removeBtn.className = 'btn btn-remove';
      removeBtn.type = 'button';
      removeBtn.textContent = 'Remove';
      (function(folderName, itemName, itemSource, itemPath) {
        removeBtn.onclick = function(e) {
          e.stopPropagation();
          if (!folderName && !itemPath) { showMessage('Remove', 'Cannot remove: folder name is missing.'); return; }
          var rb = 'Remove <strong>' + (itemName || folderName).replace(/</g, '&lt;') + '</strong> from your game folder?';
          showConfirm('Remove workshop item', rb, function() {
            var ex = getExternal();
            if (!ex) { showMessage('Remove', 'Workshop remove is not available.'); return; }
            var done = false;
            if (itemSource === 'steam' && itemPath) {
              try { ex.workshopRemoveByPath(itemPath); done = true; } catch (e1) {}
              if (!done) try { ex.invoke('workshopRemoveByPath', itemPath); done = true; } catch (e2) {}
            } else {
              try { ex.workshopRemove(folderName); done = true; } catch (e1) {}
              if (!done) try { ex.invoke('workshopRemove', folderName); done = true; } catch (e2) {}
            }
            if (done) startRemoveProgressPoll();
            else showMessage('Remove', 'Workshop remove is not available.');
          });
        };
      })(item.folder || '', item.name || item.folder, item.source || '', item.path || '');
      actions.appendChild(removeBtn);

      var body = document.createElement('div');
      body.className = 'mod-card-body';
      body.appendChild(header);
      if (descDiv) body.appendChild(descDiv);
      if (hasMeta) body.appendChild(metaDiv);
      body.appendChild(actions);

      if (item.image && String(item.image).length > 0) {
        var imgWrap = document.createElement('div');
        imgWrap.className = 'mod-card-image-wrap';
        var img = document.createElement('img');
        img.src = item.image;
        img.alt = '';
        img.onerror = (function(wrap) { return function() { if (wrap && wrap.parentNode) wrap.parentNode.removeChild(wrap); }; })(imgWrap);
        imgWrap.appendChild(img);
        card.appendChild(imgWrap);
      }

      card.appendChild(body);
      card.onclick = (function(fp) {
        return function(e) { if (fp) { try { window.external.openUrl(fp); } catch (err) {} } };
      })(item.path || '');
      modsGrid.appendChild(card);
    }

    if (modsPagination) {
      if (totalPages > 1) {
        modsPagination.style.display = 'flex';
        if (modsPageLabel) modsPageLabel.textContent = 'Page ' + modsPage + ' / ' + totalPages;
        if (modsPrevPageBtn) modsPrevPageBtn.disabled = modsPage <= 1;
        if (modsNextPageBtn) modsNextPageBtn.disabled = modsPage >= totalPages;
      } else {
        modsPagination.style.display = 'none';
      }
    }
  }

  modsGrid.onclick = function(e) {
    var t = e.target;
    while (t && t !== modsGrid) {
      var url = t.getAttribute && t.getAttribute('data-open-url');
      if (url) { e.preventDefault(); e.stopPropagation(); try { window.external.openUrl(url); } catch (err) {} return; }
      t = t.parentNode;
    }
  };

  if (modsPrevPageBtn) {
    modsPrevPageBtn.onclick = function() { if (modsPage > 1) { modsPage--; renderModsPage(); } };
  }

  if (modsNextPageBtn) {
    modsNextPageBtn.onclick = function() {
      var tp = Math.ceil((modsItemsCache || []).length / modsPageSize) || 1;
      if (modsPage < tp) { modsPage++; renderModsPage(); }
    };
  }

  document.getElementById('workshopRefreshBtn').onclick = function() { refreshModsGrid(); };

  document.getElementById('deleteAllModsBtn').onclick = function() {
    var items = modsItemsCache || [];
    var lh = '<p style="margin:0 0 8px 0;">Remove <strong>' + items.length + '</strong> workshop item' + (items.length !== 1 ? 's' : '') + ' from your game folder? This cannot be undone.</p>';
    if (items.length > 0) {
      lh += '<ul class="confirm-item-list">';
      for (var ci = 0; ci < items.length; ci++) {
        var cItem = items[ci];
        var cName = (cItem.name || cItem.folder || 'Unknown').replace(/</g, '&lt;');
        var cSz2 = parseInt(cItem.localSize, 10) || parseInt(cItem.file_size, 10) || 0;
        var cSzStr = '';
        if (cSz2 > 0) {
          cSzStr = cSz2 > 1073741824 ? (cSz2 / 1073741824).toFixed(2) + ' GB'
            : cSz2 > 1048576 ? (cSz2 / 1048576).toFixed(1) + ' MB'
            : cSz2 > 1024 ? (cSz2 / 1024).toFixed(0) + ' KB' : cSz2 + ' B';
        }
        lh += '<li><span class="item-name">' + cName + '</span>' + (cSzStr ? '<span class="item-size">' + cSzStr + '</span>' : '') + '</li>';
      }
      lh += '</ul>';
    }
    showConfirm('Delete all mods', lh, function() {
      var ex = getExternal();
      if (!ex) return;
      var done = false;
      try { ex.workshopRemoveAll(); done = true; } catch (e1) {}
      if (!done) try { ex.invoke('workshopRemoveAll', ''); done = true; } catch (e2) {}
      if (done) startRemoveProgressPoll();
    });
  };

  document.getElementById('checkUpdatesBtn').onclick = function() {
    var wsStatus = document.getElementById('workshopStatus');
    try {
      var ex = getExternal();
      if (!ex || !ex.workshopList) { if (wsStatus) wsStatus.textContent = 'Cannot check updates.'; return; }
      document.getElementById('checkUpdatesBtn').disabled = true;
      if (ex.workshopListAsync) ex.workshopListAsync();
      var cpCount = 0;
      var checkPoll = setInterval(function() {
        cpCount++;
        if (cpCount > 300) {
          clearInterval(checkPoll);
          document.getElementById('checkUpdatesBtn').disabled = false;
          return;
        }
        try {
          var ex2 = getExternal();
          if (!ex2) return;
          var isLoading = ex2.workshopListIsLoading && ex2.workshopListIsLoading() === 'true';
          if (!isLoading) {
            clearInterval(checkPoll);
            var list = ex2.workshopList();
            if (!list || list.length === 0) { document.getElementById('checkUpdatesBtn').disabled = false; return; }
            var items2 = typeof list === 'string' ? JSON.parse(list) : list;
            modsItemsCache = items2;
            modsPage = 1;
            renderModsPage();
            var toUpdate = [];
            for (var ui = 0; ui < items2.length; ui++) {
              if (items2[ui].needsUpdate && items2[ui].id && String(items2[ui].id).length > 0) toUpdate.push(items2[ui]);
            }
            if (toUpdate.length === 0) {
              document.getElementById('checkUpdatesBtn').disabled = false;
              showMessage('Updates', 'All items are up to date!');
              return;
            }
            workshopProgress.style.display = 'block';
            workshopDownloadBtn.disabled = true;
            var idx = 0;
            function updateNext() {
              if (idx >= toUpdate.length) {
                workshopDownloadBtn.disabled = false;
                document.getElementById('checkUpdatesBtn').disabled = false;
                setTimeout(refreshModsGrid, 500);
                return;
              }
              var it = toUpdate[idx];
              try { ex2.workshopDownload(String(it.id)); } catch (e) {}
              var upi = setInterval(function() {
                pollWorkshopStatus();
                var ex3 = getExternal();
                if (ex3 && ex3.workshopGetStatus) {
                  var sj = ex3.workshopGetStatus();
                  if (sj) {
                    var st = typeof sj === 'string' ? JSON.parse(sj) : sj;
                    if (st.message && (st.message.indexOf('Done') !== -1 || st.message.indexOf('Error') !== -1 || st.message.indexOf('Already installed') !== -1)) {
                      clearInterval(upi);
                      idx++;
                      setTimeout(updateNext, 300);
                    }
                  }
                }
              }, 500);
            }
            updateNext();
          }
        } catch (e) { document.getElementById('checkUpdatesBtn').disabled = false; }
      }, 100);
    } catch (e) {
      document.getElementById('checkUpdatesBtn').disabled = false;
    }
  };

  document.getElementById('manageInstallBtn').onclick = function() {
    if (manageInstallPopup) manageInstallPopup.classList.add('active');
    loadComponents();
  };

  function hideManagePopup() { if (manageInstallPopup) manageInstallPopup.classList.remove('active'); }
  manageInstallPopup.querySelector('.popup-close').onclick = hideManagePopup;
  document.getElementById('popupCancel').onclick = hideManagePopup;
  manageInstallPopup.onclick = function(e) { if (e.target === manageInstallPopup) hideManagePopup(); };

  var componentSelection = { campaign: false, multiplayer: false, zombies: false };
  var modeFilesInfo = {};

  function loadComponents() {
    var list = document.getElementById('componentsList');
    list.innerHTML = '<div style="text-align:center;padding:16px;color:rgba(200,196,192,0.5);">Scanning files...</div>';
    componentSelection = { campaign: false, multiplayer: false, zombies: false };
    try {
      var ex = getExternal();
      if (ex && ex.getModeFilesInfo) {
        var raw = ex.getModeFilesInfo();
        modeFilesInfo = typeof raw === 'string' ? JSON.parse(raw) : raw;
      } else { modeFilesInfo = {}; }
    } catch (e) { modeFilesInfo = {}; }
    var items = [
      { id: 'campaign', name: 'Campaign', prefix: 'cp_' },
      { id: 'multiplayer', name: 'Multiplayer', prefix: 'mp_' },
      { id: 'zombies', name: 'Zombies', prefix: 'zm_' }
    ];
    list.innerHTML = '';
    for (var i = 0; i < items.length; i++) {
      var it = items[i];
      var info = modeFilesInfo[it.id] || { count: 0, size: 0, sizeHuman: '0 B' };
      var installed = info.count > 0;
      var sizeText = installed ? info.sizeHuman + ' (' + info.count + ' files)' : 'Not installed';
      var div = document.createElement('div');
      div.className = 'component-item';
      if (!installed) div.style.opacity = '0.5';
      div.innerHTML = '<div class="component-checkbox">'
        + '<input type="checkbox" id="comp-' + it.id + '" data-comp="' + it.id + '"' + (installed ? '' : ' disabled') + '>'
        + '<span class="checkbox-visual"></span></div>'
        + '<div class="component-info"><div class="component-name">' + it.name + '</div>'
        + '<span class="component-size">' + sizeText + '</span></div>';
      list.appendChild(div);
      if (installed) {
        var cb = div.querySelector('input[type="checkbox"]');
        cb.onchange = function() { componentSelection[this.getAttribute('data-comp')] = this.checked; };
        div.onclick = function(e) {
          if (e.target.type !== 'checkbox') {
            var inp = this.querySelector('input[type="checkbox"]');
            if (inp && !inp.disabled) { inp.checked = !inp.checked; componentSelection[inp.getAttribute('data-comp')] = inp.checked; }
          }
        };
      }
    }
  }

  document.getElementById('popupApply').onclick = function() {
    var selected = [];
    var names = [];
    var prefixMap = { campaign: 'cp_', multiplayer: 'mp_', zombies: 'zm_' };
    var nameMap = { campaign: 'Campaign', multiplayer: 'Multiplayer', zombies: 'Zombies' };
    for (var key in componentSelection) {
      if (componentSelection[key] && modeFilesInfo[key] && modeFilesInfo[key].count > 0) {
        selected.push(prefixMap[key]);
        names.push(nameMap[key] + ' (' + modeFilesInfo[key].sizeHuman + ')');
      }
    }
    if (selected.length === 0) { showMessage('Delete', 'No game modes selected or selected modes have no files.'); return; }
    hideManagePopup();
    showConfirm('Delete Game Modes', 'Are you sure you want to remove ' + names.join(', ') + '? This cannot be undone.', function() {
      try {
        var ex = getExternal();
        if (ex && ex.removeModeFiles) {
          var result = ex.removeModeFiles(selected.join(','));
          var r = typeof result === 'string' ? JSON.parse(result) : result;
          var msg = 'Removed ' + r.removedCount + ' files (' + r.removedSize + ')';
          if (r.failedCount > 0) msg += '\nFailed to remove ' + r.failedCount + ' files';
          showMessage('Delete Complete', msg);
        }
      } catch (e) { showMessage('Error', 'Failed to remove files: ' + e.message); }
    });
  };

  (function() {
    var verifyPopup = document.getElementById('verifyPopup');
    var verifyStartBtn = document.getElementById('verifyStartBtn');
    var verifyCancelBtn = document.getElementById('verifyCancelBtn');
    var verifyProgressSection = document.getElementById('verifyProgressSection');
    var verifyStatusMsg = document.getElementById('verifyStatusMsg');
    var verifyProgressFill2 = document.getElementById('verifyProgressFill');
    var verifyProgressPct = document.getElementById('verifyProgressPct');
    var verifyStatusDetails = document.getElementById('verifyStatusDetails');
    var verifyResults = document.getElementById('verifyResults');
    var verifyCloseBtn = document.getElementById('verifyPopupClose');
    var verifyModeBtns = document.querySelectorAll('.verify-mode-btn');
    var selectedVerifyMode = 'all';
    var verifyPollInterval = null;
    var verifySeenRunning = false;
    var verifyMaxPct = 0;

    function resetVerifyUI() {
      verifyProgressSection.classList.remove('active');
      verifyResults.classList.remove('active');
      verifyResults.innerHTML = '';
      verifyStartBtn.style.display = '';
      verifyStartBtn.textContent = 'Start Verification';
      verifyCancelBtn.style.display = 'none';
      verifyProgressFill2.style.width = '0%';
      verifyProgressPct.textContent = '0%';
      verifyStatusMsg.textContent = '';
      verifyStatusMsg.style.color = '';
      verifyStatusDetails.textContent = '';
      verifySeenRunning = false;
      verifyMaxPct = 0;
      for (var i = 0; i < verifyModeBtns.length; i++) verifyModeBtns[i].disabled = false;
    }

    function showVerifyPopup() { resetVerifyUI(); verifyPopup.classList.add('active'); }
    function hideVerifyPopup() {
      if (verifyPollInterval) { clearInterval(verifyPollInterval); verifyPollInterval = null; }
      verifyPopup.classList.remove('active');
      resetVerifyUI();
    }

    function showVerifyResults(changedFiles, message) {
      verifyResults.innerHTML = '';
      verifyResults.classList.add('active');
      if (!changedFiles || changedFiles.length === 0) {
        verifyResults.innerHTML = '<div class="verify-results-ok">&#10004; All files verified — no changes detected.</div>';
      } else {
        var html = '<div style="color:rgba(249,115,22,0.9);font-size:0.78rem;font-weight:600;margin-bottom:7px;">' + changedFiles.length + ' file' + (changedFiles.length > 1 ? 's' : '') + ' with issues:</div>';
        html += '<div class="verify-results-bad">';
        for (var i = 0; i < changedFiles.length; i++) {
          var raw = changedFiles[i];
          var isWarning = raw.indexOf('[warning]') === 0;
          var display = raw.replace(/^\[(warning|error)\]\s*/, '');
          var color = isWarning ? 'color:rgba(234,179,8,0.88)' : 'color:rgba(239,68,68,0.88)';
          html += '<div class="verify-file-item" style="' + color + '">' + escapeHtml(display) + '</div>';
        }
        html += '</div>';
        verifyResults.innerHTML = html;
      }
    }

    document.getElementById('verifyBtn').onclick = function() { showVerifyPopup(); };

    verifyCloseBtn.onclick = function() {
      try { var ex = getExternal(); if (ex && ex.cancelVerify) ex.cancelVerify(); } catch (e) {}
      hideVerifyPopup();
    };

    verifyPopup.onclick = function(e) {
      if (e.target === verifyPopup) {
        try { var ex = getExternal(); if (ex && ex.cancelVerify) ex.cancelVerify(); } catch (e2) {}
        hideVerifyPopup();
      }
    };

    for (var vi = 0; vi < verifyModeBtns.length; vi++) {
      (function(btn) {
        btn.onclick = function() {
          for (var j = 0; j < verifyModeBtns.length; j++) verifyModeBtns[j].classList.remove('selected');
          btn.classList.add('selected');
          selectedVerifyMode = btn.getAttribute('data-mode');
        };
      })(verifyModeBtns[vi]);
    }

    verifyCancelBtn.onclick = function() {
      try { var ex = getExternal(); if (ex && ex.cancelVerify) ex.cancelVerify(); } catch (e) {}
      verifyCancelBtn.disabled = true;
    };

    function startVerify() {
      try {
        var ex = getExternal();
        if (!ex) { showMessage('Error', 'Verification not available.'); return; }
        var result = ex.verifyGameFiles(selectedVerifyMode);
        if (result === 'already_running') { showMessage('Verify', 'Verification is already running.'); return; }
        verifySeenRunning = false;
        verifyResults.classList.remove('active');
        verifyResults.innerHTML = '';
        verifyStartBtn.style.display = 'none';
        verifyCancelBtn.style.display = '';
        verifyCancelBtn.disabled = false;
        verifyProgressSection.classList.add('active');
        for (var k = 0; k < verifyModeBtns.length; k++) verifyModeBtns[k].disabled = true;

        verifyPollInterval = setInterval(function() {
          try {
            var ex2 = getExternal();
            if (!ex2 || !ex2.getVerifyStatus) return;
            var raw = ex2.getVerifyStatus();
            var st = typeof raw === 'string' ? JSON.parse(raw) : raw;
            if (st.running) verifySeenRunning = true;
            if (!st.running && st.progress >= 100) verifySeenRunning = true;
            if (!st.running && !verifySeenRunning && st.message && st.message.length > 0) {
              clearInterval(verifyPollInterval); verifyPollInterval = null;
              verifyCancelBtn.style.display = 'none';
              verifyStatusMsg.textContent = st.message;
              verifyStatusMsg.style.color = '#ef4444';
              verifyStartBtn.style.display = '';
              verifyStartBtn.textContent = 'Close';
              verifyStartBtn.onclick = function() { hideVerifyPopup(); verifyStartBtn.onclick = startVerify; verifyStartBtn.textContent = 'Start Verification'; };
              for (var k = 0; k < verifyModeBtns.length; k++) verifyModeBtns[k].disabled = false;
              return;
            }
            if (!verifySeenRunning) return;
            var rawPct = Math.min(st.progress || 0, 100);
            if (rawPct > verifyMaxPct) verifyMaxPct = rawPct;
            var pct = verifyMaxPct;
            verifyStatusMsg.textContent = st.message || '';
            if (st.message && (st.message.indexOf('missing') !== -1 || st.message.indexOf('wrong size') !== -1 || st.message.indexOf('Not installed') !== -1 || st.message.indexOf('No files found') !== -1)) {
              verifyStatusMsg.style.color = '#ef4444';
            } else { verifyStatusMsg.style.color = ''; }
            verifyProgressFill2.style.width = pct.toFixed(1) + '%';
            verifyProgressPct.textContent = pct.toFixed(1) + '%';
            verifyStatusDetails.textContent = st.details || '';
            if (!st.running && st.progress >= 100) {
              clearInterval(verifyPollInterval); verifyPollInterval = null;
              verifyCancelBtn.style.display = 'none';
              verifyProgressFill2.style.width = '100%';
              verifyProgressPct.textContent = '100%';
              var isBaseline = st.message && st.message.indexOf('Baseline') !== -1;
              if (isBaseline) {
                verifyResults.classList.add('active');
                verifyResults.innerHTML = '<div class="verify-results-ok">&#10004; ' + escapeHtml(st.message) + '</div>';
              } else { showVerifyResults(st.changedFiles || [], st.message); }
              for (var k = 0; k < verifyModeBtns.length; k++) verifyModeBtns[k].disabled = false;
              verifyStartBtn.textContent = 'Verify Again';
              verifyStartBtn.style.display = '';
              verifyStartBtn.onclick = startVerify;
            } else if (!st.running && st.message && (st.message.indexOf('Cancelled') !== -1 || st.message.indexOf('No game files') !== -1)) {
              clearInterval(verifyPollInterval); verifyPollInterval = null;
              verifyCancelBtn.style.display = 'none';
              verifyStartBtn.style.display = '';
              verifyStartBtn.textContent = 'Start Verification';
              verifyStartBtn.onclick = startVerify;
              for (var k = 0; k < verifyModeBtns.length; k++) verifyModeBtns[k].disabled = false;
            }
          } catch (e) {}
        }, 200);
      } catch (e) { showMessage('Error', 'Failed to start verification: ' + (e.message || '')); }
    }

    verifyStartBtn.onclick = startVerify;
  })();

  document.getElementById('playBtn').onclick = function() {
    var playBtn = document.getElementById('playBtn');
    try {
      var ex = getExternal();
      if (ex && ex.isGameRunning && ex.isGameRunning() === '1') {
        showMessage('Game Running', 'Black Ops III is already running.');
        return;
      }
      playBtn.disabled = true;
      var opts = window.getSelectedLaunchOption();
      var hasKeepLauncher = opts.toLowerCase().indexOf('keeplauncher') !== -1;
      if (window._workshopPollInterval && !hasKeepLauncher) {
        showConfirm('Download in progress', 'A workshop download is in progress. Keep the launcher open while you play?', function() {
          try { window.external.launchGame(window.getPlayerName(), opts); } catch (e2) {}
        });
        return;
      }
      if (hasKeepLauncher) { window.external.launchGame(window.getPlayerName(), opts); return; }
      window.external.runGame(window.getPlayerName(), opts);
    } catch (e) {}
  };

  setInterval(function() {
    var playBtn = document.getElementById('playBtn');
    if (!playBtn) return;
    try {
      var ex = getExternal();
      if (ex && ex.isGameRunning && ex.isGameRunning() === '1') {
        playBtn.disabled = true;
        playBtn.title = 'Game is running';
      } else {
        playBtn.disabled = false;
        playBtn.title = '';
      }
    } catch (e) {}
  }, 1500);

  var _folderPickerGlobalLock = false;
  function setupPathBtn(btnId, pathDisplayIds) {
    var btn = document.getElementById(btnId);
    if (!btn) return;
    btn.onclick = function() {
      if (_folderPickerGlobalLock) return;
      _folderPickerGlobalLock = true;
      try {
        var ex = getExternal();
        if (!ex || !ex.selectGameFolder) { _folderPickerGlobalLock = false; return; }
        var result = ex.selectGameFolder();
        _folderPickerGlobalLock = false;
        if (result === 'cancelled') return;
        if (result === 'invalid') { showMessage('Invalid Folder', 'The selected folder does not contain BlackOps3.exe.'); return; }
        if (result === 'error') { showMessage('Error', 'Failed to open folder picker.'); return; }
        for (var di = 0; di < pathDisplayIds.length; di++) {
          var el = document.getElementById(pathDisplayIds[di]);
          if (el) el.textContent = result;
        }
        showMessage('Game Path Updated', 'Game path set to:\n' + result + '\n\nRestart BOIII for the change to take full effect.');
      } catch (e) { _folderPickerGlobalLock = false; showMessage('Error', 'Failed to set game path: ' + e.message); }
    };
  }

  setupPathBtn('setPathBtn', ['currentPathDisplay', 'settingsPathDisplay']);
  setupPathBtn('settingsChangePathBtn', ['currentPathDisplay', 'settingsPathDisplay']);

  var settingsTabs = document.querySelectorAll('.settings-tab');
  var settingsTabContents = document.querySelectorAll('.settings-tab-content');
  for (var sti = 0; sti < settingsTabs.length; sti++) {
    (function(tab) {
      tab.onclick = function() {
        var target = tab.getAttribute('data-stab');
        for (var j = 0; j < settingsTabs.length; j++) settingsTabs[j].classList.remove('active');
        for (var j = 0; j < settingsTabContents.length; j++) settingsTabContents[j].classList.remove('active');
        tab.classList.add('active');
        var contentEl = document.querySelector('.settings-tab-content[data-stab="' + target + '"]');
        if (contentEl) contentEl.classList.add('active');
      };
    })(settingsTabs[sti]);
  }

  var SETTINGS_DEFAULTS = {
    closeLauncher: true,
    autoUpdate: true,
    autoRefresh: true,
    assetLimits: true,
    reduceStutter: false,
    skipIntro: false,
    skipAllIntros: false,
    friendsOnly: false,
    vsync: false,
    drawFps: false,
    smoothFramerate: false,
    unlockGfx: false,
    fullVram: false
  };

  function loadAllSettings() {
    var settings = {};
    try {
      var stored = localStorage.getItem('boiii_settings');
      if (stored) settings = JSON.parse(stored);
    } catch (e) {}
    var merged = {};
    for (var k in SETTINGS_DEFAULTS) {
      merged[k] = settings.hasOwnProperty(k) ? settings[k] : SETTINGS_DEFAULTS[k];
    }
    return merged;
  }

  function saveAllSettings(settings) {
    try { localStorage.setItem('boiii_settings', JSON.stringify(settings)); } catch (e) {}
  }

  function getSetting(key) {
    var s = loadAllSettings();
    return s.hasOwnProperty(key) ? s[key] : SETTINGS_DEFAULTS[key];
  }

  function setSetting(key, value) {
    var s = loadAllSettings();
    s[key] = value;
    saveAllSettings(s);
  }

  function applySettingsToUI() {
    var s = loadAllSettings();
    var allToggles = document.querySelectorAll('.settings-toggle[data-setting]');
    for (var i = 0; i < allToggles.length; i++) {
      var key = allToggles[i].getAttribute('data-setting');
      if (s[key]) {
        allToggles[i].classList.add('active');
      } else {
        allToggles[i].classList.remove('active');
      }
    }
  }

  var allSettingToggles = document.querySelectorAll('.settings-toggle[data-setting]');
  for (var ti = 0; ti < allSettingToggles.length; ti++) {
    (function(toggle) {
      toggle.onclick = function() {
        toggle.classList.toggle('active');
        var key = toggle.getAttribute('data-setting');
        var isActive = toggle.classList.contains('active');
        setSetting(key, isActive);
        onSettingChanged(key, isActive);
      };
    })(allSettingToggles[ti]);
  }

  function onSettingChanged(key, value) {
    try {
      var ex = getExternal();
      if (!ex) return;
      if (key === 'reduceStutter' && ex.toggleReduceStutter) {
        ex.toggleReduceStutter(value ? '1' : '0');
      } else if (key === 'skipIntro' && ex.toggleSkipIntro) {
        ex.toggleSkipIntro(value ? '1' : '0');
      } else if (key === 'skipAllIntros' && ex.toggleSkipAllIntros) {
        ex.toggleSkipAllIntros(value ? '1' : '0');
        if (value) {
          setSetting('skipIntro', true);
          var siToggle = document.getElementById('settingsSkipIntro');
          if (siToggle) siToggle.classList.add('active');
        }
      } else if (key === 'friendsOnly' && ex.setGameSetting) {
        ex.setGameSetting('friendsOnly', value ? '1' : '0');
      } else if (key === 'vsync' && ex.setGameSetting) {
        ex.setGameSetting('Vsync', value ? '1' : '0');
      } else if (key === 'drawFps' && ex.setGameSetting) {
        ex.setGameSetting('DrawFPS', value ? '1' : '0');
      } else if (key === 'smoothFramerate' && ex.setGameSetting) {
        ex.setGameSetting('SmoothFramerate', value ? '1' : '0');
      } else if (key === 'unlockGfx' && ex.setGameSetting) {
        ex.setGameSetting('RestrictGraphicsOptions', value ? '0' : '1');
      } else if (key === 'fullVram' && ex.setGameSetting) {
        if (value) {
          ex.setGameSetting('VideoMemory', '1');
          ex.setGameSetting('StreamMinResident', '0');
        } else {
          ex.setGameSetting('VideoMemory', '0.75');
          ex.setGameSetting('StreamMinResident', '1');
        }
      } else if (key === 'assetLimits') {
        if (ex.setGameSetting) ex.setGameSetting('asset_limits_enabled', value ? '1' : '0');
      }
    } catch (e) {}
  }

  applySettingsToUI();

  function setupSettingsInput(inputId, configKey) {
    var input = document.getElementById(inputId);
    if (!input) return;
    input.onchange = function() {
      var val = input.value.replace(/^\s+|\s+$/g, '');
      if (!val) return;
      try {
        var ex = getExternal();
        if (ex && ex.setGameSetting) ex.setGameSetting(configKey, val);
      } catch (e) {}
    };
  }

  setupSettingsInput('settingsResolution', 'WindowSize');
  setupSettingsInput('settingsRefreshRate', 'RefreshRate');
  setupSettingsInput('settingsFpsLimit', 'MaxFPS');
  setupSettingsInput('settingsFov', 'FOV');
  setupSettingsInput('settingsMaxLatency', 'MaxFrameLatency');

  var displayModeSelect = document.getElementById('settingsDisplayMode');
  if (displayModeSelect) {
    displayModeSelect.onchange = function() {
      try {
        var ex = getExternal();
        if (ex && ex.setGameSetting) ex.setGameSetting('FullScreenMode', displayModeSelect.value);
      } catch (e) {}
    };
  }

  var networkPwInput = document.getElementById('settingsNetworkPassword');
  if (networkPwInput) {
    networkPwInput.onchange = function() {
      try {
        var ex = getExternal();
        if (ex && ex.setGameSetting) ex.setGameSetting('networkpassword', networkPwInput.value);
      } catch (e) {}
    };
  }

  var applyPresetBtn = document.getElementById('settingsApplyPresetBtn');
  if (applyPresetBtn) {
    applyPresetBtn.onclick = function() {
      var presetSelect = document.getElementById('settingsGraphicsPreset');
      if (!presetSelect || !presetSelect.value) return;
      try {
        var ex = getExternal();
        if (ex && ex.applyGraphicsPreset) {
          var result = ex.applyGraphicsPreset(presetSelect.value);
          if (result === 'ok') {
            showMessage('Preset Applied', 'Graphics preset applied successfully. Restart the game for changes to take effect.');
            loadGameSettings();
          } else {
            showMessage('Error', 'Failed to apply preset: ' + result);
          }
        }
      } catch (e) {}
    };
  }

  function loadGameSettings() {
    try {
      var ex = getExternal();
      if (!ex || !ex.getGameSettings) return;
      var json = ex.getGameSettings();
      if (!json) return;
      var s = typeof json === 'string' ? JSON.parse(json) : json;
      if (!s) return;

      if (s.FullScreenMode !== undefined && displayModeSelect) displayModeSelect.value = String(s.FullScreenMode);
      if (s.WindowSize !== undefined) { var el = document.getElementById('settingsResolution'); if (el) el.value = s.WindowSize; }
      if (s.RefreshRate !== undefined) { var el = document.getElementById('settingsRefreshRate'); if (el) el.value = s.RefreshRate; }
      if (s.Vsync !== undefined) {
        setSetting('vsync', s.Vsync === '1' || s.Vsync === 1);
        var vt = document.getElementById('settingsVsync');
        if (vt) { if (s.Vsync === '1' || s.Vsync === 1) vt.classList.add('active'); else vt.classList.remove('active'); }
      }

      if (s.MaxFPS !== undefined) { var el = document.getElementById('settingsFpsLimit'); if (el) el.value = s.MaxFPS; }
      if (s.FOV !== undefined) { var el = document.getElementById('settingsFov'); if (el) el.value = s.FOV; }
      if (s.DrawFPS !== undefined) {
        setSetting('drawFps', s.DrawFPS === '1' || s.DrawFPS === 1);
        var dt = document.getElementById('settingsDrawFps');
        if (dt) { if (s.DrawFPS === '1' || s.DrawFPS === 1) dt.classList.add('active'); else dt.classList.remove('active'); }
      }
      if (s.SmoothFramerate !== undefined) {
        setSetting('smoothFramerate', s.SmoothFramerate === '1' || s.SmoothFramerate === 1);
        var sf = document.getElementById('settingsSmoothFramerate');
        if (sf) { if (s.SmoothFramerate === '1' || s.SmoothFramerate === 1) sf.classList.add('active'); else sf.classList.remove('active'); }
      }

      if (s.RestrictGraphicsOptions !== undefined) {
        var unlocked = s.RestrictGraphicsOptions === '0' || s.RestrictGraphicsOptions === 0;
        setSetting('unlockGfx', unlocked);
        var ut = document.getElementById('settingsUnlockGfx');
        if (ut) { if (unlocked) ut.classList.add('active'); else ut.classList.remove('active'); }
      }
      if (s.MaxFrameLatency !== undefined) { var el = document.getElementById('settingsMaxLatency'); if (el) el.value = s.MaxFrameLatency; }
      if (s.VideoMemory !== undefined) {
        var fv = (s.VideoMemory === '1' || s.VideoMemory === 1) && (s.StreamMinResident === '0' || s.StreamMinResident === 0);
        setSetting('fullVram', fv);
        var ft = document.getElementById('settingsFullVram');
        if (ft) { if (fv) ft.classList.add('active'); else ft.classList.remove('active'); }
      }

      if (s.networkpassword !== undefined && networkPwInput) networkPwInput.value = s.networkpassword;
      if (s.friendsOnly !== undefined) {
        var fo = s.friendsOnly === '1' || s.friendsOnly === 1 || s.friendsOnly === true;
        setSetting('friendsOnly', fo);
        var ft2 = document.getElementById('settingsFriendsOnly');
        if (ft2) { if (fo) ft2.classList.add('active'); else ft2.classList.remove('active'); }
      }

      if (s.reduceStutter !== undefined) {
        setSetting('reduceStutter', s.reduceStutter === true || s.reduceStutter === '1');
        var rst = document.getElementById('settingsReduceStutter');
        if (rst) { if (s.reduceStutter === true || s.reduceStutter === '1') rst.classList.add('active'); else rst.classList.remove('active'); }
      }
      if (s.skipIntro !== undefined) {
        setSetting('skipIntro', s.skipIntro === true || s.skipIntro === '1');
        var sit = document.getElementById('settingsSkipIntro');
        if (sit) { if (s.skipIntro === true || s.skipIntro === '1') sit.classList.add('active'); else sit.classList.remove('active'); }
      }
      if (s.skipAllIntros !== undefined) {
        setSetting('skipAllIntros', s.skipAllIntros === true || s.skipAllIntros === '1');
        var sat = document.getElementById('settingsSkipAllIntros');
        if (sat) { if (s.skipAllIntros === true || s.skipAllIntros === '1') sat.classList.add('active'); else sat.classList.remove('active'); }
      }

      if (s.assetLimits !== undefined) {
        var al = s.assetLimits === '1' || s.assetLimits === 1 || s.assetLimits === true;
        setSetting('assetLimits', al);
        var alt = document.getElementById('settingsAssetLimits');
        if (alt) { if (al) alt.classList.add('active'); else alt.classList.remove('active'); }
      }
    } catch (e) {}
  }

  loadGameSettings();

  var resetBtn = document.getElementById('settingsResetBtn');
  if (resetBtn) {
    resetBtn.onclick = function() {
      showConfirm('Reset Settings', 'Are you sure you want to reset all settings to defaults? This cannot be undone.', function() {
        try {
          var ex = getExternal();
          // Revert file-backed toggles
          if (ex) {
            if (ex.toggleReduceStutter) ex.toggleReduceStutter('0');
            if (ex.toggleSkipIntro) ex.toggleSkipIntro('0');
            if (ex.toggleSkipAllIntros) ex.toggleSkipAllIntros('0');
            if (ex.setGameSetting) {
              ex.setGameSetting('asset_limits_enabled', '1');
              ex.setGameSetting('friendsOnly', '0');
              ex.setGameSetting('networkpassword', '');
            }
          }
        } catch (e) {}
        try { localStorage.removeItem('boiii_settings'); } catch (e) {}
        for (var k in SETTINGS_DEFAULTS) {
          setSetting(k, SETTINGS_DEFAULTS[k]);
        }
        applySettingsToUI();
        if (networkPwInput) networkPwInput.value = '';
        loadGameSettings();
        showMessage('Settings Reset', 'All settings have been reset to defaults. Restart the game for changes to take effect.');
      });
    };
  }


  var workshopProgressFolder = document.getElementById('workshopProgressFolder');
  if (workshopProgressFolder) {
    workshopProgressFolder.onclick = function() {
      if (_workshopDownloadFolder && _workshopDownloadFolder.length > 0) {
        try { window.external.openUrl(_workshopDownloadFolder); } catch (e) {}
      }
    };
  }

  refreshModsGrid();
})();
