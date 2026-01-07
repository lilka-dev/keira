#include "web.h"
#include "esp_http_server.h"
#include <keira/servicemanager.h>

// Modern CSS styles inspired by catalog.lilka.dev and lilka-dev.github.io/flasher
static const char cssStyles[] = R"rawliteral(
<style>
:root {
  --bg-primary: #1a1b26;
  --bg-secondary: #24283b;
  --bg-tertiary: #414868;
  --accent-primary: #7aa2f7;
  --accent-secondary: #bb9af7;
  --accent-success: #9ece6a;
  --accent-warning: #e0af68;
  --accent-danger: #f7768e;
  --text-primary: #c0caf5;
  --text-secondary: #a9b1d6;
  --text-muted: #565f89;
  --border-color: #414868;
  --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.3);
}
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
  background: linear-gradient(135deg, var(--bg-primary) 0%, #16161e 100%);
  color: var(--text-primary);
  min-height: 100vh;
  padding: 20px;
}
.container { max-width: 900px; margin: 0 auto; }
header { text-align: center; padding: 30px 0; margin-bottom: 30px; border-bottom: 1px solid var(--border-color); }
.logo { font-size: 2.5em; margin-bottom: 10px; }
h1 { color: var(--accent-primary); font-size: 1.8em; margin-bottom: 5px; }
.subtitle { color: var(--text-muted); font-size: 0.95em; }
.card {
  background: var(--bg-secondary);
  border-radius: 12px;
  padding: 24px;
  margin-bottom: 20px;
  box-shadow: var(--shadow);
  border: 1px solid var(--border-color);
}
.card-title { font-size: 1.2em; color: var(--accent-secondary); margin-bottom: 16px; display: flex; align-items: center; gap: 10px; }
.btn {
  display: inline-flex; align-items: center; justify-content: center; gap: 8px;
  padding: 12px 24px; border-radius: 8px; font-size: 0.95em; font-weight: 500;
  cursor: pointer; transition: all 0.2s ease; border: none; text-decoration: none;
}
.btn-primary { background: linear-gradient(135deg, var(--accent-primary) 0%, #5a8dee 100%); color: white; }
.btn-primary:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(122, 162, 247, 0.4); }
.btn-success { background: linear-gradient(135deg, var(--accent-success) 0%, #76b947 100%); color: #1a1b26; }
.btn-success:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(158, 206, 106, 0.4); }
.btn-danger { background: linear-gradient(135deg, var(--accent-danger) 0%, #e05668 100%); color: white; }
.btn-danger:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(247, 118, 142, 0.4); }
.btn-secondary { background: var(--bg-tertiary); color: var(--text-primary); border: 1px solid var(--border-color); }
.btn-secondary:hover { background: var(--border-color); transform: translateY(-2px); }
.file-input-wrapper { position: relative; display: flex; flex-direction: column; gap: 12px; }
.file-input { display: none; }
.drop-zone {
  display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 10px;
  padding: 40px 20px; border: 2px dashed var(--border-color); border-radius: 12px;
  cursor: pointer; transition: all 0.2s ease; text-align: center; color: var(--text-secondary);
}
.drop-zone:hover, .drop-zone.drag-over { border-color: var(--accent-primary); background: rgba(122, 162, 247, 0.1); }
.drop-zone.drag-over { border-style: solid; transform: scale(1.02); }
.file-name { padding: 12px; background: var(--bg-tertiary); border-radius: 8px; font-size: 0.9em; color: var(--accent-success); display: none; }
.file-name.active { display: block; }
.nav-links { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 12px; }
.nav-link {
  display: flex; align-items: center; gap: 12px; padding: 16px 20px;
  background: var(--bg-tertiary); border-radius: 10px; text-decoration: none;
  color: var(--text-primary); transition: all 0.2s ease; border: 1px solid transparent;
}
.nav-link:hover { border-color: var(--accent-primary); background: rgba(122, 162, 247, 0.15); transform: translateX(4px); }
.nav-link .icon { font-size: 1.5em; }
.nav-link .text { display: flex; flex-direction: column; }
.nav-link .title { font-weight: 500; }
.nav-link .desc { font-size: 0.8em; color: var(--text-muted); }
.progress-container { display: none; margin-top: 16px; }
.progress-container.active { display: block; }
.progress-bar { height: 8px; background: var(--bg-tertiary); border-radius: 4px; overflow: hidden; }
.progress-fill { height: 100%; background: linear-gradient(90deg, var(--accent-primary), var(--accent-secondary)); border-radius: 4px; transition: width 0.3s ease; width: 0%; }
.progress-text { margin-top: 8px; font-size: 0.85em; color: var(--text-muted); text-align: center; }
.status { padding: 12px 16px; border-radius: 8px; margin-top: 16px; display: none; }
.status.active { display: block; }
.status.success { background: rgba(158, 206, 106, 0.2); color: var(--accent-success); }
.status.error { background: rgba(247, 118, 142, 0.2); color: var(--accent-danger); }
footer { text-align: center; padding: 30px 0; margin-top: 30px; border-top: 1px solid var(--border-color); color: var(--text-muted); font-size: 0.85em; }
footer a { color: var(--accent-primary); text-decoration: none; }
footer a:hover { text-decoration: underline; }
@media (max-width: 600px) { body { padding: 12px; } .card { padding: 16px; } .btn { padding: 10px 16px; font-size: 0.9em; } }
</style>
)rawliteral";

static const char htmlHead[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Keira Web Manager</title>
)rawliteral";

static const char htmlBody[] = R"rawliteral(
</head>
<body>
  <div class="container">
    <header>
      <div class="logo">&#127800;</div>
      <h1>Keira Web Manager</h1>
      <p class="subtitle">File &amp; Firmware Management for Lilka</p>
    </header>

    <div class="card">
      <div class="card-title">&#9889; Flash OTA Firmware (download !NOT! Merged .bin file from github releases)</div>
      <form id="otaForm" action="/firmware" method="post" enctype="multipart/form-data">
        <div class="file-input-wrapper">
          <input type="file" name="file" id="otaFile" class="file-input" accept=".bin">
          <label for="otaFile" class="drop-zone" id="otaDrop">
            <span>&#128193; Drop .bin file here or click to select</span>
            <span style="font-size:0.8em;color:var(--text-muted)">For OTA firmware update</span>
          </label>
          <div id="otaFileName" class="file-name"></div>
          <button type="submit" class="btn btn-danger">&#9889; Flash OTA &amp; Reboot</button>
        </div>
        <div id="otaProgress" class="progress-container">
          <div class="progress-bar"><div id="otaProgressFill" class="progress-fill"></div></div>
          <div id="otaProgressText" class="progress-text">Uploading...</div>
        </div>
        <div id="otaStatus" class="status"></div>
      </form>
    </div>

    <div class="card">
      <div class="card-title">&#128194; Browse Files</div>
      <div class="nav-links">
        <a href="/download?sd=true" class="nav-link">
          <span class="icon">&#128190;</span>
          <span class="text">
            <span class="title">SD Card</span>
            <span class="desc">Browse SD card files</span>
          </span>
        </a>
      </div>
    </div>

    <footer>
      <p>&#127800; <a href="https://lilka.dev" target="_blank">lilka.dev</a> &#8226; <a href="https://github.com/lilka-dev/keira" target="_blank">GitHub</a> &#8226; <a href="https://docs.lilka.dev" target="_blank">Docs</a></p>
    </footer>
  </div>

  <script>
    function formatSize(bytes) {
      if (bytes < 1024) return bytes + ' B';
      if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
      return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
    }

    function setupDropZone(dropId, inputId, nameId) {
      var drop = document.getElementById(dropId);
      var input = document.getElementById(inputId);
      var nameEl = document.getElementById(nameId);

      ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(function(evt) {
        drop.addEventListener(evt, function(e) { e.preventDefault(); e.stopPropagation(); });
      });
      ['dragenter', 'dragover'].forEach(function(evt) {
        drop.addEventListener(evt, function() { drop.classList.add('drag-over'); });
      });
      ['dragleave', 'drop'].forEach(function(evt) {
        drop.addEventListener(evt, function() { drop.classList.remove('drag-over'); });
      });

      drop.addEventListener('drop', function(e) {
        var files = e.dataTransfer.files;
        if (files.length > 0) {
          input.files = files;
          updateName(input, nameEl);
        }
      });

      input.addEventListener('change', function() { updateName(input, nameEl); });
    }

    function updateName(input, nameEl) {
      if (input.files && input.files[0]) {
        nameEl.innerHTML = '&#128196; ' + input.files[0].name + ' (' + formatSize(input.files[0].size) + ')';
        nameEl.classList.add('active');
      } else {
        nameEl.classList.remove('active');
      }
    }

    function setupForm(formId, progressId, fillId, textId, statusId) {
      var form = document.getElementById(formId);
      form.addEventListener('submit', function(e) {
        e.preventDefault();
        var formData = new FormData(form);
        var xhr = new XMLHttpRequest();
        var progress = document.getElementById(progressId);
        var fill = document.getElementById(fillId);
        var text = document.getElementById(textId);
        var status = document.getElementById(statusId);

        progress.classList.add('active');
        status.classList.remove('active', 'success', 'error');

        xhr.upload.onprogress = function(ev) {
          if (ev.lengthComputable) {
            var pct = Math.round((ev.loaded / ev.total) * 100);
            fill.style.width = pct + '%';
            text.textContent = pct + '% (' + formatSize(ev.loaded) + ' / ' + formatSize(ev.total) + ')';
          }
        };

        xhr.onload = function() {
          progress.classList.remove('active');
          status.classList.add('active');
          if (xhr.status >= 200 && xhr.status < 300) {
            status.classList.add('success');
            status.textContent = 'Success!';
            if (formId === 'otaForm') status.textContent += ' Device rebooting...';
            else if (formId === 'multibootForm') status.textContent += ' Booting app...';
          } else {
            status.classList.add('error');
            status.textContent = 'Error: ' + (xhr.responseText || xhr.statusText);
          }
        };

        xhr.onerror = function() {
          progress.classList.remove('active');
          status.classList.add('active', 'error');
          status.textContent = 'Connection error';
        };

        xhr.open('POST', form.action, true);
        xhr.send(formData);
      });
    }

    setupDropZone('otaDrop', 'otaFile', 'otaFileName');
    setupForm('otaForm', 'otaProgress', 'otaProgressFill', 'otaProgressText', 'otaStatus');
  </script>
</body>
</html>
)rawliteral";

static const char htmlListHead[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>File Browser - Keira</title>
)rawliteral";

static const char cssFileList[] = R"rawliteral(
<style>
.file-list { display: flex; flex-direction: column; gap: 4px; }
.file-item {
  display: flex; align-items: center; gap: 12px; padding: 12px 16px;
  background: var(--bg-tertiary); border-radius: 8px; text-decoration: none;
  color: var(--text-primary); transition: all 0.2s ease; border: 1px solid transparent;
}
.file-item:hover { border-color: var(--accent-primary); background: rgba(122, 162, 247, 0.15); transform: translateX(4px); }
.file-item .icon { font-size: 1.3em; min-width: 24px; text-align: center; }
.file-item .name { flex: 1; word-break: break-all; }
.file-item.folder .icon { color: var(--accent-warning); }
.file-item.file .icon { color: var(--text-muted); }
.file-item.bin .icon { color: var(--accent-success); }
.file-item.lua .icon { color: var(--accent-primary); }
.file-item.nes .icon { color: var(--accent-danger); }
.file-actions { display: flex; gap: 8px; }
.file-actions .action-btn {
  padding: 6px 12px; border-radius: 6px; font-size: 0.8em;
  text-decoration: none; cursor: pointer; transition: all 0.2s ease;
}
.action-btn.boot { background: var(--accent-success); color: #1a1b26; }
.action-btn.preview { background: var(--accent-primary); color: white; }
.action-btn.download { background: var(--bg-tertiary); color: var(--text-primary); border: 1px solid var(--border-color); }
.action-btn.delete { background: var(--accent-danger); color: white; }
.action-btn.copy { background: var(--accent-warning); color: #1a1b26; }
.action-btn:hover { transform: scale(1.05); }
.modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.7); z-index: 1000; align-items: center; justify-content: center; }
.modal.active { display: flex; }
.modal-content { background: var(--bg-secondary); border-radius: 12px; padding: 24px; max-width: 500px; width: 90%; border: 1px solid var(--border-color); }
.modal-title { font-size: 1.3em; color: var(--accent-primary); margin-bottom: 16px; }
.modal-body { margin-bottom: 20px; }
.modal-input { width: 100%; padding: 10px; background: var(--bg-tertiary); border: 1px solid var(--border-color); border-radius: 6px; color: var(--text-primary); font-size: 1em; margin-top: 8px; }
.modal-actions { display: flex; gap: 12px; justify-content: flex-end; }
.folder-tree { max-height: 300px; overflow-y: auto; border: 1px solid var(--border-color); border-radius: 6px; padding: 8px; background: var(--bg-tertiary); }
.folder-item { padding: 8px; cursor: pointer; border-radius: 4px; transition: all 0.2s; }
.folder-item:hover { background: var(--bg-primary); }
.folder-item.selected { background: var(--accent-primary); color: white; }
.toolbar { display: flex; gap: 8px; margin-bottom: 12px; }
.toolbar-btn { padding: 10px 16px; background: var(--bg-tertiary); border: 1px solid var(--border-color); border-radius: 6px; color: var(--text-primary); cursor: pointer; transition: all 0.2s; }
.toolbar-btn:hover { border-color: var(--accent-primary); background: rgba(122, 162, 247, 0.15); }
#opProgress { display: none; margin-top: 12px; }
.parent-link { background: var(--bg-secondary) !important; border: 1px dashed var(--border-color) !important; }
.file-item.image .icon { color: var(--accent-secondary); }
.file-item.video .icon { color: var(--accent-danger); }
.file-item.audio .icon { color: var(--accent-warning); }
.file-item.text .icon { color: var(--text-secondary); }
.file-list-drop-zone { padding: 20px; border: 2px dashed var(--border-color); border-radius: 8px; text-align: center; color: var(--text-muted); margin-bottom: 12px; transition: all 0.2s ease; }
.file-list-drop-zone.drag-over { border-color: var(--accent-primary); background: rgba(122, 162, 247, 0.1); border-style: solid; }
.file-list-drop-zone .icon { font-size: 2em; margin-bottom: 8px; }
</style>
)rawliteral";

static const char htmlListBodyBegin[] = R"rawliteral(
</head>
<body>
  <div class="container">
    <header>
      <div class="logo">&#128194;</div>
      <h1>File Browser</h1>
      <p class="subtitle" id="currentPath"></p>
    </header>
    <div class="card">
      <div class="card-title">&#128193; Files</div>
      <div class="file-list">
)rawliteral";

static const char htmlListBodyEnd[] = R"rawliteral(
      </div>
    </div>
    <div class="card">
      <a href="/" class="btn btn-secondary" style="width: 100%; text-align: center;">&#8592; Back to Home</a>
    </div>
    <footer>
      <p>&#127800; <a href="https://lilka.dev" target="_blank">lilka.dev</a></p>
    </footer>
  </div>
</body>
</html>
)rawliteral";

static const char bootScript[] = R"rawliteral(
<script>
var bootProgressInterval = null;
function bootFile(path) {
  if(confirm('Boot this file via Multiboot?\n' + path)) {
    fetch('/boot?p=' + encodeURIComponent(path), {method: 'POST'})
      .then(function(r) {
        if (r.ok) {
          showProgress('Booting application...');
          startBootProgressPolling();
        } else {
          r.text().then(function(t) { alert('Error: ' + t); });
        }
      })
      .catch(function(e) { alert('Error: ' + e); });
  }
}
function startBootProgressPolling() {
  if (bootProgressInterval) clearInterval(bootProgressInterval);
  bootProgressInterval = setInterval(function() {
    fetch('/progress')
      .then(function(r) { return r.json(); })
      .then(function(data) {
        var progress = data.progress;
        if (progress >= 0 && progress <= 100) {
          var prog = document.getElementById('opProgress');
          if (prog) {
            var fill = document.getElementById('opProgressFill');
            var text = document.getElementById('opProgressText');
            if (fill) fill.style.width = progress + '%';
            if (text) text.textContent = 'Booting: ' + progress + '%';
            prog.style.display = 'block';
          }
          if (progress === 100) {
            clearInterval(bootProgressInterval);
            bootProgressInterval = null;
            setTimeout(function() { hideProgress(); alert('Rebooting device...'); }, 2000);
          }
        } else if (progress === -2) {
          clearInterval(bootProgressInterval);
          bootProgressInterval = null;
          hideProgress();
          alert('Boot failed!');
        }
      })
      .catch(function() {});
  }, 500);
}
function previewFile(path, sd) {
  window.location.href = '/preview?p=' + encodeURIComponent(path) + (sd ? '&sd=true' : '');
}
var dropZone = document.getElementById('fileDropZone');
if (dropZone) {
  ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(function(evt) {
    dropZone.addEventListener(evt, function(e) { e.preventDefault(); e.stopPropagation(); });
  });
  ['dragenter', 'dragover'].forEach(function(evt) {
    dropZone.addEventListener(evt, function() { dropZone.classList.add('drag-over'); });
  });
  ['dragleave', 'drop'].forEach(function(evt) {
    dropZone.addEventListener(evt, function() { dropZone.classList.remove('drag-over'); });
  });
  dropZone.addEventListener('drop', function(e) {
    var files = e.dataTransfer.files;
    if (files.length > 0) {
      for (var i = 0; i < files.length; i++) {
        uploadFileToFolder(files[i], currentPath, isSdCard);
      }
    }
  });
}
function uploadFileToFolder(file, folder, sd) {
  var formData = new FormData();
  formData.append('file', file);
  var url = '/upload' + (sd ? '?sd=true' : '?sd=false');
  if (folder && folder.length > 0) url += '&path=' + encodeURIComponent(folder);
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url, true);
  xhr.onload = function() {
    if (xhr.status >= 200 && xhr.status < 300) {
      alert('File uploaded: ' + file.name);
      location.reload();
    } else {
      alert('Upload failed: ' + xhr.responseText);
    }
  };
  xhr.onerror = function() { alert('Upload error'); };
  xhr.send(formData);
}
function closeModal(id) {
  document.getElementById(id).classList.remove('active');
}
function showNewFolderDialog() {
  document.getElementById('newFolderName').value = '';
  document.getElementById('newFolderModal').classList.add('active');
}
function createNewFolder() {
  var name = document.getElementById('newFolderName').value.trim();
  if (!name) { alert('Please enter a folder name'); return; }
  var url = '/mkdir?sd=' + (isSdCard ? 'true' : 'false') + '&p=' + encodeURIComponent(currentPath + '/' + name);
  fetch(url, {method: 'POST'})
    .then(function(r) { return r.ok ? (closeModal('newFolderModal'), location.reload()) : r.text().then(function(t) { alert('Error: ' + t); }); })
    .catch(function(e) { alert('Error: ' + e); });
}
function deleteFile(path, sd) {
  if (!confirm('Delete: ' + path + '?')) return;
  showProgress('Deleting...');
  fetch('/delete?sd=' + (sd ? 'true' : 'false') + '&p=' + encodeURIComponent(path), {method: 'POST'})
    .then(function(r) { return r.ok ? (hideProgress(), location.reload()) : r.text().then(function(t) { hideProgress(); alert('Error: ' + t); }); })
    .catch(function(e) { hideProgress(); alert('Error: ' + e); });
}
var copySourcePath = '';
var copySourceSd = false;
function showCopyDialog(path, sd) {
  copySourcePath = path;
  copySourceSd = sd;
  document.getElementById('copyFileName').textContent = 'File: ' + path;
  loadFolderTree();
  document.getElementById('copyModal').classList.add('active');
}
function loadFolderTree() {
  var tree = document.getElementById('folderTree');
  tree.innerHTML = '<div class=\"folder-item selected\" data-path=\"\">/ (Root)</div>';
  fetch('/listdirs?sd=' + (isSdCard ? 'true' : 'false') + '&p=' + encodeURIComponent(currentPath))
    .then(function(r) { return r.json(); })
    .then(function(data) {
      data.forEach(function(folder) {
        var item = document.createElement('div');
        item.className = 'folder-item';
        item.textContent = '  ' + folder;
        item.dataset.path = folder;
        item.onclick = function() {
          tree.querySelectorAll('.folder-item').forEach(function(el) { el.classList.remove('selected'); });
          item.classList.add('selected');
        };
        tree.appendChild(item);
      });
    })
    .catch(function(e) { tree.innerHTML = '<div>Error loading folders</div>'; });
}
function executeCopy() {
  var selected = document.querySelector('#folderTree .folder-item.selected');
  if (!selected) { alert('Please select a destination folder'); return; }
  var destPath = selected.dataset.path;
  closeModal('copyModal');
  showProgress('Copying...');
  fetch('/copy?sd=' + (copySourceSd ? 'true' : 'false') + '&src=' + encodeURIComponent(copySourcePath) + '&dst=' + encodeURIComponent(destPath), {method: 'POST'})
    .then(function(r) { return r.ok ? (hideProgress(), location.reload()) : r.text().then(function(t) { hideProgress(); alert('Error: ' + t); }); })
    .catch(function(e) { hideProgress(); alert('Error: ' + e); });
}
function showProgress(msg) {
  var prog = document.getElementById('opProgress');
  document.getElementById('opProgressText').textContent = msg;
  document.getElementById('opProgressFill').style.width = '50%';
  prog.style.display = 'block';
}
function hideProgress() {
  document.getElementById('opProgress').style.display = 'none';
}
</script>
)rawliteral";

static const char htmlPreviewHead[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Preview - Keira</title>
)rawliteral";

static const char cssPreview[] = R"rawliteral(
<style>
.preview-container { max-width: 100%; margin: 20px 0; }
.preview-container img { max-width: 100%; height: auto; border-radius: 8px; display: block; margin: 0 auto; }
.preview-container video { max-width: 100%; height: auto; border-radius: 8px; display: block; margin: 0 auto; background: #000; }
.preview-container audio { width: 100%; margin: 20px 0; }
.preview-container pre { background: var(--bg-tertiary); padding: 16px; border-radius: 8px; overflow-x: auto; white-space: pre-wrap; word-wrap: break-word; max-height: 70vh; overflow-y: auto; font-family: monospace; font-size: 0.9em; line-height: 1.5; }
.preview-container iframe { width: 100%; height: 70vh; border: none; border-radius: 8px; background: white; }
.preview-info { display: flex; gap: 16px; flex-wrap: wrap; margin-bottom: 16px; padding: 12px; background: var(--bg-tertiary); border-radius: 8px; font-size: 0.9em; }
.preview-info span { color: var(--text-muted); }
.preview-info strong { color: var(--text-primary); }
.preview-actions { display: flex; gap: 12px; flex-wrap: wrap; margin-top: 16px; }
.unsupported { text-align: center; padding: 40px; color: var(--text-muted); }
.unsupported .icon { font-size: 3em; margin-bottom: 16px; }
</style>
)rawliteral";

static const char htmlPreviewBodyBegin[] = R"rawliteral(
</head>
<body>
  <div class="container">
    <header>
      <div class="logo">&#128065;</div>
      <h1>File Preview</h1>
      <p class="subtitle" id="fileName"></p>
    </header>
    <div class="card">
      <div class="preview-info" id="fileInfo"></div>
      <div class="preview-container" id="previewArea">
)rawliteral";

static const char htmlPreviewBodyEnd[] = R"rawliteral(
      </div>
      <div class="preview-actions" id="actions"></div>
    </div>
    <div class="card">
      <a href="javascript:history.back()" class="btn btn-secondary" style="width: 100%; text-align: center;">&#8592; Back</a>
    </div>
    <footer>
      <p>&#127800; <a href="https://lilka.dev" target="_blank">lilka.dev</a></p>
    </footer>
  </div>
</body>
</html>
)rawliteral";

static httpd_handle_t stream_httpd = NULL;
static const char* contentLengthHeader = "Content-Length";
static const char* fileHeaderDivider = "\r\n\r\n";
static volatile bool pendingRestart = false;
static volatile bool pendingMultiboot = false;
static String pendingMultibootPath = "";
static volatile int multibootProgress = -1; // -1=idle, -2=error, 0-100=progress

static esp_err_t index_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html; charset=UTF-8");
    httpd_resp_sendstr_chunk(req, htmlHead);
    httpd_resp_sendstr_chunk(req, cssStyles);
    httpd_resp_sendstr_chunk(req, htmlBody);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static int getContentLength(httpd_req_t* req) {
    char contentLength[64];
    auto err = httpd_req_get_hdr_value_str(req, contentLengthHeader, contentLength, sizeof(contentLength));
    if (err != ESP_OK) {
        return -1;
    }
    char* endptr;
    return strtol(contentLength, &endptr, 10);
}

String urlDecode(const String& str) {
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = str.length();
    for (unsigned int i = 0; i < len; i++) {
        char c = str.charAt(i);
        if (c == '+') {
            decoded += ' ';
        } else if (c == '%' && i + 2 < len) {
            temp[2] = str.charAt(i + 1);
            temp[3] = str.charAt(i + 2);
            decoded += (char)strtol(temp, NULL, 16);
            i += 2;
        } else {
            decoded += c;
        }
    }
    return decoded;
}

String getQueryPath(httpd_req_t* req, bool* sdCardSelected) {
    const int queryStringSize = 512;
    char queryString[queryStringSize];
    String result;

    auto err = httpd_req_get_url_query_str(req, queryString, queryStringSize);
    if (err == ESP_OK) {
        auto query = strnstr(queryString, "p=", queryStringSize);
        if (query != NULL) {
            query += 2;
            auto queryEnd = strchrnul(query, '&');
            result = urlDecode(String(query, queryEnd - query));
        }
        sdCardSelected[0] = strnstr(queryString, "sd=true", queryStringSize) != NULL;
    } else {
        sdCardSelected[0] = false;
    }
    return result;
}

static bool isImageFile(const String& filename) {
    return filename.endsWith(".jpg") || filename.endsWith(".jpeg") || filename.endsWith(".png") ||
           filename.endsWith(".gif") || filename.endsWith(".bmp") || filename.endsWith(".webp");
}

static bool isVideoFile(const String& filename) {
    return filename.endsWith(".mp4") || filename.endsWith(".webm") || filename.endsWith(".avi") ||
           filename.endsWith(".mov");
}

static bool isAudioFile(const String& filename) {
    return filename.endsWith(".mp3") || filename.endsWith(".wav") || filename.endsWith(".ogg") ||
           filename.endsWith(".flac") || filename.endsWith(".mod") || filename.endsWith(".lt");
}

static bool isTextFile(const String& filename) {
    return filename.endsWith(".txt") || filename.endsWith(".json") || filename.endsWith(".xml") ||
           filename.endsWith(".lua") || filename.endsWith(".js") || filename.endsWith(".css") ||
           filename.endsWith(".html") || filename.endsWith(".htm") || filename.endsWith(".md") ||
           filename.endsWith(".ini") || filename.endsWith(".cfg") || filename.endsWith(".conf") ||
           filename.endsWith(".log") || filename.endsWith(".csv");
}

static bool isPdfFile(const String& filename) {
    return filename.endsWith(".pdf");
}

static bool isPreviewable(const String& filename) {
    String lower = filename;
    lower.toLowerCase();
    return isImageFile(lower) || isVideoFile(lower) || isAudioFile(lower) || isTextFile(lower) || isPdfFile(lower);
}

static const char* getFileClass(const char* name) {
    String filename = String(name);
    filename.toLowerCase();
    if (filename.endsWith(".bin")) return "bin";
    if (filename.endsWith(".lua")) return "lua";
    if (filename.endsWith(".nes")) return "nes";
    if (isImageFile(filename)) return "image";
    if (isVideoFile(filename)) return "video";
    if (isAudioFile(filename)) return "audio";
    if (isTextFile(filename)) return "text";
    return "file";
}

static const char* getFileIcon(const char* name, bool isDir) {
    if (isDir) return "&#128193;";
    String filename = String(name);
    filename.toLowerCase();
    if (filename.endsWith(".bin")) return "&#9889;";
    if (filename.endsWith(".lua")) return "&#127769;";
    if (filename.endsWith(".nes")) return "&#127918;";
    if (filename.endsWith(".js")) return "&#128220;";
    if (filename.endsWith(".mod") || filename.endsWith(".lt")) return "&#127925;";
    if (isImageFile(filename)) return "&#128247;";
    if (isVideoFile(filename)) return "&#127909;";
    if (isAudioFile(filename)) return "&#127911;";
    if (isTextFile(filename)) return "&#128196;";
    if (isPdfFile(filename)) return "&#128213;";
    return "&#128196;";
}

static const char* getMimeType(const String& filename) {
    String lower = filename;
    lower.toLowerCase();
    if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) return "image/jpeg";
    if (lower.endsWith(".png")) return "image/png";
    if (lower.endsWith(".gif")) return "image/gif";
    if (lower.endsWith(".bmp")) return "image/bmp";
    if (lower.endsWith(".webp")) return "image/webp";
    if (lower.endsWith(".mp4")) return "video/mp4";
    if (lower.endsWith(".webm")) return "video/webm";
    if (lower.endsWith(".avi")) return "video/x-msvideo";
    if (lower.endsWith(".mov")) return "video/quicktime";
    if (lower.endsWith(".mp3")) return "audio/mpeg";
    if (lower.endsWith(".wav")) return "audio/wav";
    if (lower.endsWith(".ogg")) return "audio/ogg";
    if (lower.endsWith(".flac")) return "audio/flac";
    if (lower.endsWith(".pdf")) return "application/pdf";
    if (lower.endsWith(".json")) return "application/json";
    if (lower.endsWith(".xml")) return "application/xml";
    if (lower.endsWith(".html") || lower.endsWith(".htm")) return "text/html";
    if (lower.endsWith(".css")) return "text/css";
    if (lower.endsWith(".js")) return "application/javascript";
    return "text/plain";
}

static esp_err_t replyWithDirectory(httpd_req_t* req, DIR* dir, const String& query, bool sdCardSelected) {
    lilka::serial.log("List dir %s", query.c_str());
    esp_err_t err = httpd_resp_set_type(req, "text/html; charset=UTF-8");
    if (err != ESP_OK) return err;

    const struct dirent* direntry = NULL;
    httpd_resp_sendstr_chunk(req, htmlListHead);
    httpd_resp_sendstr_chunk(req, cssStyles);
    httpd_resp_sendstr_chunk(req, cssFileList);
    httpd_resp_sendstr_chunk(req, htmlListBodyBegin);

    String pathScript = "<script>";
    pathScript += "currentPath='";
    pathScript += query;
    pathScript += "';isSdCard=";
    pathScript += sdCardSelected ? "true" : "false";
    pathScript += ";document.getElementById('currentPath').textContent='";
    pathScript += sdCardSelected ? "SD Card: /" : "SPIFFS: /";
    pathScript += query;
    pathScript += "';</script>";
    httpd_resp_sendstr_chunk(req, pathScript.c_str());

    String toolbarHtml = "<div class='toolbar'>";
    toolbarHtml += "<button class='toolbar-btn' onclick='showNewFolderDialog()'>&#128193; New Folder</button>";
    toolbarHtml += "</div>";
    httpd_resp_sendstr_chunk(req, toolbarHtml.c_str());

    String dropZoneHtml = "<div class='file-list-drop-zone' id='fileDropZone'><div "
                          "class='icon'>&#128229;</div><div>Drop files here to upload to current folder</div></div>";
    dropZoneHtml +=
        "<div id='opProgress' class='progress-container'><div class='progress-bar'><div id='opProgressFill' "
        "class='progress-fill'></div></div><div id='opProgressText' class='progress-text'></div></div>";
    httpd_resp_sendstr_chunk(req, dropZoneHtml.c_str());

    if (query.length() > 0) {
        String parentPath = query;
        int lastSlash = parentPath.lastIndexOf('/');
        parentPath = (lastSlash > 0) ? parentPath.substring(0, lastSlash) : "";

        String parentHtml = "<a href='/download?";
        if (sdCardSelected) parentHtml += "sd=true&";
        parentHtml += "p=";
        parentHtml += parentPath;
        parentHtml += "' class='file-item folder parent-link'>";
        parentHtml += "<span class='icon'>&#11014;</span>";
        parentHtml += "<span class='name'>.. (Parent Directory)</span></a>";
        httpd_resp_sendstr_chunk(req, parentHtml.c_str());
    }

    while ((direntry = readdir(dir)) != NULL) {
        bool isDir = direntry->d_type == DT_DIR;
        auto absolutePath = lilka::fileutils.joinPath(query, direntry->d_name);
        const char* fileClass = isDir ? "folder" : getFileClass(direntry->d_name);
        const char* fileIcon = getFileIcon(direntry->d_name, isDir);

        String itemHtml = "<a href='/download?";
        if (sdCardSelected) itemHtml += "sd=true&";
        itemHtml += "p=";
        itemHtml += absolutePath;
        itemHtml += "' class='file-item ";
        itemHtml += fileClass;
        itemHtml += "'><span class='icon'>";
        itemHtml += fileIcon;
        itemHtml += "</span><span class='name'>";
        itemHtml += direntry->d_name;
        itemHtml += "</span>";

        if (!isDir) {
            String filename = String(direntry->d_name);
            filename.toLowerCase();
            itemHtml += "<div class='file-actions'>";
            if (isPreviewable(filename)) {
                itemHtml += "<span class='action-btn preview' onclick='event.preventDefault();previewFile(\"";
                itemHtml += absolutePath;
                itemHtml += "\",";
                itemHtml += sdCardSelected ? "true" : "false";
                itemHtml += ")'>&#128065;</span>";
            }
            if (sdCardSelected && filename.endsWith(".bin")) {
                itemHtml += "<span class='action-btn boot' onclick='event.preventDefault();bootFile(\"";
                itemHtml += absolutePath;
                itemHtml += "\")'>&#128640;</span>";
            }
            itemHtml += "<span class='action-btn copy' onclick='event.preventDefault();showCopyDialog(\"";
            itemHtml += absolutePath;
            itemHtml += "\",";
            itemHtml += sdCardSelected ? "true" : "false";
            itemHtml += ")'>&#128203;</span>";
            itemHtml += "<span class='action-btn delete' onclick='event.preventDefault();deleteFile(\"";
            itemHtml += absolutePath;
            itemHtml += "\",";
            itemHtml += sdCardSelected ? "true" : "false";
            itemHtml += ")'>&#128465;</span>";
            itemHtml += "</div>";
        } else {
            itemHtml += "<div class='file-actions'>";
            itemHtml += "<span class='action-btn delete' onclick='event.preventDefault();deleteFile(\"";
            itemHtml += absolutePath;
            itemHtml += "\",";
            itemHtml += sdCardSelected ? "true" : "false";
            itemHtml += ")'>&#128465;</span>";
            itemHtml += "</div>";
        }
        itemHtml += "</a>";
        httpd_resp_sendstr_chunk(req, itemHtml.c_str());
    }

    String modalsHtml =
        R"(<div class='modal' id='newFolderModal'><div class='modal-content'><div class='modal-title'>&#128193; Create New Folder</div><div class='modal-body'><input type='text' id='newFolderName' class='modal-input' placeholder='Folder name'></div><div class='modal-actions'><button class='btn btn-secondary' onclick='closeModal("newFolderModal")'>Cancel</button><button class='btn btn-primary' onclick='createNewFolder()'>Create</button></div></div></div><div class='modal' id='copyModal'><div class='modal-content'><div class='modal-title'>&#128203; Copy File</div><div class='modal-body'><div id='copyFileName' style='margin-bottom:12px;color:var(--text-secondary);'></div><label>Destination folder:</label><div class='folder-tree' id='folderTree'></div></div><div class='modal-actions'><button class='btn btn-secondary' onclick='closeModal("copyModal")'>Cancel</button><button class='btn btn-primary' onclick='executeCopy()'>Copy</button></div></div></div>)";
    httpd_resp_sendstr_chunk(req, modalsHtml.c_str());
    httpd_resp_sendstr_chunk(req, bootScript);
    httpd_resp_sendstr_chunk(req, htmlListBodyEnd);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t replyWithFile(httpd_req_t* req, const String& path) {
    esp_err_t err = ESP_OK;
    String fileNameHeader;

    auto file = fopen(path.c_str(), "r");
    if (file == NULL) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    for (auto scanForName = path.end(); scanForName > path.begin(); scanForName--) {
        if (*scanForName == '/' || *scanForName == '\\') {
            scanForName++;
            fileNameHeader = "attachment; filename=\"";
            fileNameHeader += scanForName;
            fileNameHeader += "\"";
            err = httpd_resp_set_hdr(req, "Content-Disposition", fileNameHeader.c_str());
            break;
        }
    }

    if (err == ESP_OK) {
        char* fileBuf = static_cast<char*>(malloc(WEB_BUFFER_FS_OP));
        err = httpd_resp_set_type(req, "application/octet-stream");
        if (err == ESP_OK) {
            size_t readBytes;
            do {
                readBytes = fread(fileBuf, 1, WEB_BUFFER_FS_OP, file);
                err = httpd_resp_send_chunk(req, fileBuf, readBytes);
                if (err != ESP_OK) break;
            } while (readBytes > 0);
        }
        free(fileBuf);
    }
    fclose(file);
    return err;
}

static esp_err_t download_handler(httpd_req_t* req) {
    bool sdCardSelected;
    struct stat statbuf;
    statbuf.st_mode = _IFDIR;

    String query = getQueryPath(req, &sdCardSelected);
    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();
    auto path = lilka::fileutils.joinPath(root, query);

    if (stat(path.c_str(), &statbuf) != 0) {
        statbuf.st_mode = _IFDIR;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        DIR* dir = opendir(path.c_str());
        if (dir != NULL) {
            esp_err_t err = replyWithDirectory(req, dir, query, sdCardSelected);
            closedir(dir);
            return err;
        }
    }
    return replyWithFile(req, path);
}

static esp_err_t upload_handler(httpd_req_t* req) {
    String lastError = "No error";
    esp_ota_handle_t ota_handle;
    esp_err_t res = httpd_resp_set_type(req, "text/plain");
    if (res != ESP_OK) return res;

    int contentLength = getContentLength(req);
    lilka::serial.log("FW upload binary size %d", contentLength);

    auto current_partition = esp_ota_get_running_partition();
    auto ota_partition = esp_ota_get_next_update_partition(current_partition);
    esp_err_t err = esp_ota_begin(ota_partition, contentLength, &ota_handle);

    if (err == ESP_OK) {
        char* buf = static_cast<char*>(malloc(WEB_BUFFER_FLASH_OP));
        bool seekBinary = true;
        lilka::serial.log("FW upload begin");

        int len = 0;
        do {
            len = httpd_req_recv(req, buf, WEB_BUFFER_FLASH_OP);
            if (len < 0) {
                lilka::serial.log("FW upload error %d", len);
                lastError = "Failed to receive firmware file";
                break;
            }

            char* towrite = buf;
            if (seekBinary) {
                auto beforeBinary = strnstr(buf, fileHeaderDivider, len);
                if (beforeBinary) {
                    towrite = beforeBinary + 4;
                    len -= towrite - buf;
                }
                seekBinary = false;
            }

            if (len > 0) {
                err = esp_ota_write(ota_handle, towrite, len);
                if (err != ESP_OK) {
                    lastError = "Failed to write OTA partition";
                    break;
                }
            }
        } while (len > 0);

        if (err == ESP_OK) {
            err = esp_ota_end(ota_handle);
            if (err == ESP_OK) {
                err = esp_ota_set_boot_partition(ota_partition);
                if (err != ESP_OK) lastError = "Failed to set boot partition";
            } else {
                lastError = "Failed to finalize OTA";
            }
        }
        free(buf);
    } else {
        lastError = "Failed to get OTA partition";
    }

    if (err == ESP_OK) {
        res = httpd_resp_sendstr(req, "OK");
        lilka::serial.log("FW upload done. Restart scheduled");
        pendingRestart = true;
    } else {
        lilka::serial.log("FW upload failed. Error %d", (int)err);
        esp_ota_abort(ota_handle);
        httpd_resp_set_status(req, HTTPD_500);
        res = httpd_resp_sendstr(req, lastError.c_str());
    }
    return res;
}

static String extractFilename(const char* buf, int len) {
    const char* filenameStart = strnstr(buf, "filename=\"", len);
    if (filenameStart) {
        filenameStart += 10;
        const char* filenameEnd = strchr(filenameStart, '"');
        if (filenameEnd) return String(filenameStart, filenameEnd - filenameStart);
    }
    return "uploaded_file.bin";
}

static esp_err_t sd_upload_handler(httpd_req_t* req) {
    String lastError = "No error";
    esp_err_t res = httpd_resp_set_type(req, "text/plain");
    if (res != ESP_OK) return res;

    int contentLength = getContentLength(req);
    lilka::serial.log("SD upload size %d", contentLength);

    bool sdCardSelected = false;
    String targetFolder = "";
    const int queryStringSize = 512;
    char queryString[queryStringSize];
    if (httpd_req_get_url_query_str(req, queryString, queryStringSize) == ESP_OK) {
        lilka::serial.log("Query string: %s", queryString);
        sdCardSelected = strnstr(queryString, "sd=true", queryStringSize) != NULL;
        char pathValue[256] = {0};
        if (httpd_query_key_value(queryString, "path", pathValue, sizeof(pathValue)) == ESP_OK) {
            targetFolder = urlDecode(String(pathValue));
        }
    }
    lilka::serial.log("SD selected: %d, target folder: %s", sdCardSelected, targetFolder.c_str());

    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();
    if (root.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, sdCardSelected ? "SD card not available" : "SPIFFS not available");
    }

    char* buf = static_cast<char*>(malloc(WEB_BUFFER_FS_OP));
    if (!buf) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Memory allocation failed");
    }

    FILE* file = NULL;
    bool seekBinary = true;
    String filename;
    esp_err_t err = ESP_OK;

    int len = 0;
    do {
        len = httpd_req_recv(req, buf, WEB_BUFFER_FS_OP);
        if (len < 0) {
            lastError = "Failed to receive file";
            err = ESP_FAIL;
            break;
        }

        if (seekBinary) {
            filename = extractFilename(buf, len);
            lilka::serial.log("Uploading file: %s to folder: %s", filename.c_str(), targetFolder.c_str());
            String fullPath = targetFolder.length() > 0 ? lilka::fileutils.joinPath(targetFolder, filename) : filename;
            String filePath = lilka::fileutils.joinPath(root, fullPath);
            lilka::serial.log("Full file path: %s", filePath.c_str());
            file = fopen(filePath.c_str(), "wb");
            if (!file) {
                lilka::serial.log("Failed to create file: %s", filePath.c_str());
                lastError = "Failed to create file on SD card";
                err = ESP_FAIL;
                break;
            }

            auto beforeBinary = strnstr(buf, fileHeaderDivider, len);
            if (beforeBinary) {
                const char* towrite = beforeBinary + 4;
                len -= (towrite - buf);
                if (len > 0) fwrite(towrite, 1, len, file);
            }
            seekBinary = false;
        } else if (len > 0 && file) {
            fwrite(buf, 1, len, file);
        }
    } while (len > 0);

    if (file) fclose(file);
    free(buf);

    if (err == ESP_OK) {
        res = httpd_resp_sendstr(req, "OK");
    } else {
        httpd_resp_set_status(req, HTTPD_500);
        res = httpd_resp_sendstr(req, lastError.c_str());
    }
    return res;
}

static esp_err_t multiboot_upload_handler(httpd_req_t* req) {
    String lastError = "No error";
    esp_err_t res = httpd_resp_set_type(req, "text/plain");
    if (res != ESP_OK) return res;

    int contentLength = getContentLength(req);
    lilka::serial.log("Multiboot upload size %d", contentLength);

    auto sdRoot = lilka::fileutils.getSDRoot();
    if (sdRoot.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "SD card not available");
    }

    char* buf = static_cast<char*>(malloc(WEB_BUFFER_FS_OP));
    if (!buf) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Memory allocation failed");
    }

    String filePath = lilka::fileutils.joinPath(sdRoot, "_web_multiboot.bin");
    FILE* file = NULL;
    bool seekBinary = true;
    esp_err_t err = ESP_OK;

    int len = 0;
    do {
        len = httpd_req_recv(req, buf, WEB_BUFFER_FS_OP);
        if (len < 0) {
            lastError = "Failed to receive file";
            err = ESP_FAIL;
            break;
        }

        if (seekBinary) {
            file = fopen(filePath.c_str(), "wb");
            if (!file) {
                lastError = "Failed to create file on SD card";
                err = ESP_FAIL;
                break;
            }

            auto beforeBinary = strnstr(buf, fileHeaderDivider, len);
            if (beforeBinary) {
                const char* towrite = beforeBinary + 4;
                len -= (towrite - buf);
                if (len > 0) fwrite(towrite, 1, len, file);
            }
            seekBinary = false;
        } else if (len > 0 && file) {
            fwrite(buf, 1, len, file);
        }
    } while (len > 0);

    if (file) fclose(file);
    free(buf);

    if (err == ESP_OK) {
        lilka::serial.log("Multiboot file saved, scheduling boot: %s", filePath.c_str());
        pendingMultibootPath = filePath;
        pendingMultiboot = true;
        res = httpd_resp_sendstr(req, "OK");
    } else {
        httpd_resp_set_status(req, HTTPD_500);
        res = httpd_resp_sendstr(req, lastError.c_str());
    }
    return res;
}

static esp_err_t preview_handler(httpd_req_t* req) {
    bool sdCardSelected;
    String query = getQueryPath(req, &sdCardSelected);

    if (query.length() == 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_sendstr(req, "No file path specified");
    }

    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();
    String filePath = lilka::fileutils.joinPath(root, query);
    String filename = query;
    int lastSlash = filename.lastIndexOf('/');
    if (lastSlash >= 0) filename = filename.substring(lastSlash + 1);

    struct stat statbuf;
    if (stat(filePath.c_str(), &statbuf) != 0) {
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_sendstr(req, "File not found");
    }

    String lowerName = filename;
    lowerName.toLowerCase();
    const char* mimeType = getMimeType(lowerName);
    bool isImage = isImageFile(lowerName);
    bool isVideo = isVideoFile(lowerName);
    bool isAudio = isAudioFile(lowerName);
    bool isText = isTextFile(lowerName);
    bool isPdf = isPdfFile(lowerName);

    httpd_resp_set_type(req, "text/html; charset=UTF-8");
    httpd_resp_sendstr_chunk(req, htmlPreviewHead);
    httpd_resp_sendstr_chunk(req, cssStyles);
    httpd_resp_sendstr_chunk(req, cssPreview);
    httpd_resp_sendstr_chunk(req, htmlPreviewBodyBegin);

    String downloadUrl = "/download?p=" + query;
    if (sdCardSelected) downloadUrl += "&sd=true";

    if (isImage) {
        httpd_resp_sendstr_chunk(req, "<img src='");
        httpd_resp_sendstr_chunk(req, downloadUrl.c_str());
        httpd_resp_sendstr_chunk(req, "' alt='");
        httpd_resp_sendstr_chunk(req, filename.c_str());
        httpd_resp_sendstr_chunk(req, "'>");
    } else if (isVideo) {
        httpd_resp_sendstr_chunk(req, "<video controls><source src='");
        httpd_resp_sendstr_chunk(req, downloadUrl.c_str());
        httpd_resp_sendstr_chunk(req, "' type='");
        httpd_resp_sendstr_chunk(req, mimeType);
        httpd_resp_sendstr_chunk(req, "'>Your browser does not support video.</video>");
    } else if (isAudio) {
        httpd_resp_sendstr_chunk(req, "<audio controls><source src='");
        httpd_resp_sendstr_chunk(req, downloadUrl.c_str());
        httpd_resp_sendstr_chunk(req, "' type='");
        httpd_resp_sendstr_chunk(req, mimeType);
        httpd_resp_sendstr_chunk(req, "'>Your browser does not support audio.</audio>");
    } else if (isPdf) {
        httpd_resp_sendstr_chunk(req, "<iframe src='");
        httpd_resp_sendstr_chunk(req, downloadUrl.c_str());
        httpd_resp_sendstr_chunk(req, "'></iframe>");
    } else if (isText) {
        FILE* file = fopen(filePath.c_str(), "r");
        if (file) {
            httpd_resp_sendstr_chunk(req, "<pre>");
            char* buf = static_cast<char*>(malloc(256));
            if (buf) {
                size_t totalRead = 0;
                const size_t maxPreview = 32 * 1024;
                while (totalRead < maxPreview) {
                    size_t toRead = 255;
                    if (totalRead + toRead > maxPreview) toRead = maxPreview - totalRead;
                    size_t bytesRead = fread(buf, 1, toRead, file);
                    if (bytesRead == 0) break;
                    buf[bytesRead] = 0;
                    for (size_t i = 0; i < bytesRead; i++) {
                        if (buf[i] == '<') httpd_resp_sendstr_chunk(req, "&lt;");
                        else if (buf[i] == '>') httpd_resp_sendstr_chunk(req, "&gt;");
                        else if (buf[i] == '&') httpd_resp_sendstr_chunk(req, "&amp;");
                        else {
                            char c[2] = {buf[i], 0};
                            httpd_resp_sendstr_chunk(req, c);
                        }
                    }
                    totalRead += bytesRead;
                }
                if (totalRead >= maxPreview) {
                    httpd_resp_sendstr_chunk(req, "\n\n... (truncated, file too large) ...");
                }
                free(buf);
            }
            fclose(file);
            httpd_resp_sendstr_chunk(req, "</pre>");
        }
    } else {
        httpd_resp_sendstr_chunk(
            req,
            "<div class='unsupported'><div class='icon'>&#128196;</div><p>Preview not available for this file "
            "type</p></div>"
        );
    }

    httpd_resp_sendstr_chunk(req, "<script>");
    httpd_resp_sendstr_chunk(req, "document.getElementById('fileName').textContent='");
    httpd_resp_sendstr_chunk(req, filename.c_str());
    httpd_resp_sendstr_chunk(req, "';");
    httpd_resp_sendstr_chunk(req, "document.getElementById('fileInfo').innerHTML='");
    httpd_resp_sendstr_chunk(req, "<span>Size: <strong>");
    char sizeStr[32];
    if (statbuf.st_size < 1024) {
        snprintf(sizeStr, sizeof(sizeStr), "%d B", (int)statbuf.st_size);
    } else if (statbuf.st_size < 1024 * 1024) {
        snprintf(sizeStr, sizeof(sizeStr), "%.1f KB", statbuf.st_size / 1024.0);
    } else {
        snprintf(sizeStr, sizeof(sizeStr), "%.2f MB", statbuf.st_size / (1024.0 * 1024.0));
    }
    httpd_resp_sendstr_chunk(req, sizeStr);
    httpd_resp_sendstr_chunk(req, "</strong></span>");
    httpd_resp_sendstr_chunk(req, "<span>Type: <strong>");
    httpd_resp_sendstr_chunk(req, mimeType);
    httpd_resp_sendstr_chunk(req, "</strong></span>");
    httpd_resp_sendstr_chunk(req, "<span>Location: <strong>");
    httpd_resp_sendstr_chunk(req, sdCardSelected ? "SD Card" : "SPIFFS");
    httpd_resp_sendstr_chunk(req, "</strong></span>';");
    httpd_resp_sendstr_chunk(req, "document.getElementById('actions').innerHTML='");
    httpd_resp_sendstr_chunk(req, "<a href=\\\"");
    httpd_resp_sendstr_chunk(req, downloadUrl.c_str());
    httpd_resp_sendstr_chunk(req, "\\\" class=\\\"btn btn-primary\\\" download>&#128229; Download</a>';");
    httpd_resp_sendstr_chunk(req, "</script>");

    httpd_resp_sendstr_chunk(req, htmlPreviewBodyEnd);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t mkdir_handler(httpd_req_t* req) {
    bool sdCardSelected;
    String query = getQueryPath(req, &sdCardSelected);
    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();

    if (root.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, sdCardSelected ? "SD card not available" : "SPIFFS not available");
    }

    String dirPath = lilka::fileutils.joinPath(root, query);
    if (mkdir(dirPath.c_str(), 0755) == 0) {
        return httpd_resp_sendstr(req, "OK");
    } else {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Failed to create directory");
    }
}

static esp_err_t delete_handler(httpd_req_t* req) {
    bool sdCardSelected;
    String query = getQueryPath(req, &sdCardSelected);
    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();

    if (root.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, sdCardSelected ? "SD card not available" : "SPIFFS not available");
    }

    String filePath = lilka::fileutils.joinPath(root, query);
    struct stat statbuf;
    if (stat(filePath.c_str(), &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            if (rmdir(filePath.c_str()) == 0) {
                return httpd_resp_sendstr(req, "OK");
            }
        } else {
            if (unlink(filePath.c_str()) == 0) {
                return httpd_resp_sendstr(req, "OK");
            }
        }
    }
    httpd_resp_set_status(req, HTTPD_500);
    return httpd_resp_sendstr(req, "Failed to delete");
}

static esp_err_t copy_handler(httpd_req_t* req) {
    const int queryStringSize = 512;
    char queryString[queryStringSize];
    bool sdCardSelected = false;
    String srcPath, dstPath;

    if (httpd_req_get_url_query_str(req, queryString, queryStringSize) == ESP_OK) {
        sdCardSelected = strnstr(queryString, "sd=true", queryStringSize) != NULL;
        char srcValue[256] = {0}, dstValue[256] = {0};
        httpd_query_key_value(queryString, "src", srcValue, sizeof(srcValue));
        httpd_query_key_value(queryString, "dst", dstValue, sizeof(dstValue));
        srcPath = urlDecode(String(srcValue));
        dstPath = urlDecode(String(dstValue));
    }

    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();
    if (root.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Storage not available");
    }

    String srcFullPath = lilka::fileutils.joinPath(root, srcPath);
    String filename = srcPath.substring(srcPath.lastIndexOf('/') + 1);

    // Handle destination: if empty, copy to root; otherwise append filename to destination folder
    String dstFullPath;
    if (dstPath.length() == 0 || dstPath == "/") {
        dstFullPath = lilka::fileutils.joinPath(root, filename);
    } else {
        // dstPath is relative folder name from current directory
        String relativeDst = lilka::fileutils.joinPath(srcPath.substring(0, srcPath.lastIndexOf('/')), dstPath);
        dstFullPath = lilka::fileutils.joinPath(root, lilka::fileutils.joinPath(relativeDst, filename));
    }

    FILE* src = fopen(srcFullPath.c_str(), "rb");
    if (!src) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Failed to open source file");
    }

    FILE* dst = fopen(dstFullPath.c_str(), "wb");
    if (!dst) {
        fclose(src);
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "Failed to create destination file");
    }

    char buffer[512];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytesRead, dst);
    }

    fclose(src);
    fclose(dst);
    return httpd_resp_sendstr(req, "OK");
}

static esp_err_t listdirs_handler(httpd_req_t* req) {
    bool sdCardSelected;
    String query = getQueryPath(req, &sdCardSelected);
    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();

    if (root.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "[]");
    }

    String dirPath = lilka::fileutils.joinPath(root, query);
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "[]");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr_chunk(req, "[");

    const struct dirent* direntry;
    bool first = true;
    while ((direntry = readdir(dir)) != NULL) {
        if (direntry->d_type == DT_DIR && strcmp(direntry->d_name, ".") != 0 && strcmp(direntry->d_name, "..") != 0) {
            if (!first) httpd_resp_sendstr_chunk(req, ",");
            httpd_resp_sendstr_chunk(req, "\"");
            httpd_resp_sendstr_chunk(req, direntry->d_name);
            httpd_resp_sendstr_chunk(req, "\"");
            first = false;
        }
    }

    closedir(dir);
    httpd_resp_sendstr_chunk(req, "]");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t progress_handler(httpd_req_t* req) {
    char progressJson[64];
    snprintf(progressJson, sizeof(progressJson), "{\"progress\":%d}", multibootProgress);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, progressJson);
}

static esp_err_t boot_handler(httpd_req_t* req) {
    bool sdCardSelected;
    String query = getQueryPath(req, &sdCardSelected);

    if (query.length() == 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_sendstr(req, "No file path specified");
    }

    auto sdRoot = lilka::fileutils.getSDRoot();
    if (sdRoot.length() == 0) {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_sendstr(req, "SD card not available");
    }

    String filePath = lilka::fileutils.joinPath(sdRoot, query);

    struct stat statbuf;
    if (stat(filePath.c_str(), &statbuf) != 0) {
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_sendstr(req, "File not found");
    }

    String lowerPath = filePath;
    lowerPath.toLowerCase();
    if (!lowerPath.endsWith(".bin")) {
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_sendstr(req, "Only .bin files can be booted");
    }

    lilka::serial.log("Boot request for: %s", filePath.c_str());
    multibootProgress = 0;
    pendingMultibootPath = filePath;
    pendingMultiboot = true;
    return httpd_resp_sendstr(req, "OK");
}

static void stopWebServer() {
    lilka::serial.log("Stopping web service");
    httpd_stop(stream_httpd);
}

static void startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 15;

    httpd_uri_t index_uri = {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL};
    httpd_uri_t upload_fw = {.uri = "/firmware", .method = HTTP_POST, .handler = upload_handler, .user_ctx = NULL};
    httpd_uri_t download_uri = {.uri = "/download", .method = HTTP_GET, .handler = download_handler, .user_ctx = NULL};
    httpd_uri_t sd_upload_uri = {.uri = "/upload", .method = HTTP_POST, .handler = sd_upload_handler, .user_ctx = NULL};
    httpd_uri_t multiboot_uri = {
        .uri = "/multiboot", .method = HTTP_POST, .handler = multiboot_upload_handler, .user_ctx = NULL
    };
    httpd_uri_t boot_uri = {.uri = "/boot", .method = HTTP_POST, .handler = boot_handler, .user_ctx = NULL};
    httpd_uri_t preview_uri = {.uri = "/preview", .method = HTTP_GET, .handler = preview_handler, .user_ctx = NULL};
    httpd_uri_t mkdir_uri = {.uri = "/mkdir", .method = HTTP_POST, .handler = mkdir_handler, .user_ctx = NULL};
    httpd_uri_t delete_uri = {.uri = "/delete", .method = HTTP_POST, .handler = delete_handler, .user_ctx = NULL};
    httpd_uri_t copy_uri = {.uri = "/copy", .method = HTTP_POST, .handler = copy_handler, .user_ctx = NULL};
    httpd_uri_t listdirs_uri = {.uri = "/listdirs", .method = HTTP_GET, .handler = listdirs_handler, .user_ctx = NULL};
    httpd_uri_t progress_uri = {.uri = "/progress", .method = HTTP_GET, .handler = progress_handler, .user_ctx = NULL};

    lilka::serial.log("Start web service on %d", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        httpd_register_uri_handler(stream_httpd, &upload_fw);
        httpd_register_uri_handler(stream_httpd, &download_uri);
        httpd_register_uri_handler(stream_httpd, &sd_upload_uri);
        httpd_register_uri_handler(stream_httpd, &multiboot_uri);
        httpd_register_uri_handler(stream_httpd, &boot_uri);
        httpd_register_uri_handler(stream_httpd, &preview_uri);
        httpd_register_uri_handler(stream_httpd, &mkdir_uri);
        httpd_register_uri_handler(stream_httpd, &delete_uri);
        httpd_register_uri_handler(stream_httpd, &copy_uri);
        httpd_register_uri_handler(stream_httpd, &listdirs_uri);
        httpd_register_uri_handler(stream_httpd, &progress_uri);
        httpd_register_uri_handler(stream_httpd, &preview_uri);
    }
}

WebService::WebService() : Service("web") {
    networkService = ServiceManager::getInstance()->getService<NetworkService>("network");
}

WebService::~WebService() {
}

void WebService::run() {
    bool wasOnline = false;
    setStackSize(16384);

    while (true) {
        if (pendingRestart) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();
        }

        if (pendingMultiboot && pendingMultibootPath.length() > 0) {
            // Stop HTTP server before multiboot

            // Create dedicated task for multiboot (like catalog app does)
            String pathCopy = pendingMultibootPath;
            pendingMultiboot = false;
            pendingMultibootPath = "";

            xTaskCreate(
                [](void* param) {
                    String* path = static_cast<String*>(param);
                    vTaskDelay(pdMS_TO_TICKS(1000));

                    lilka::serial.log("Starting multiboot: %s", path->c_str());

                    int error = lilka::multiboot.start(*path);
                    if (error) {
                        lilka::serial.log("Multiboot start error: %d", error);
                        multibootProgress = -2;
                        delete path;
                        vTaskDelete(NULL);
                        return;
                    }

                    size_t totalBytes = lilka::multiboot.getBytesTotal();
                    lilka::serial.log("Multiboot size: %d bytes", totalBytes);

                    int lastProgress = -1;
                    while ((error = lilka::multiboot.process()) > 0) {
                        size_t written = lilka::multiboot.getBytesWritten();
                        int progress = (totalBytes > 0) ? (written * 100 / totalBytes) : 0;

                        multibootProgress = progress;

                        if (progress != lastProgress && progress % 10 == 0) {
                            lilka::serial.log("Multiboot progress: %d%% (%d/%d bytes)", progress, written, totalBytes);
                            lastProgress = progress;
                        }

                        vTaskDelay(pdMS_TO_TICKS(10));
                    }

                    if (error < 0) {
                        lilka::serial.log("Multiboot process error: %d", error);
                        multibootProgress = -2;
                        delete path;
                        vTaskDelete(NULL);
                        return;
                    }

                    multibootProgress = 100;
                    error = lilka::multiboot.finishAndReboot();
                    if (error) {
                        lilka::serial.log("Multiboot finish error: %d", error);
                        multibootProgress = -2;
                    }
                    delete path;
                    vTaskDelete(NULL);
                },
                "multiboot",
                16384,
                new String(pathCopy),
                1,
                NULL
            );
        }

        if (!networkService) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        bool isOnline = networkService->getNetworkState() == NetworkState::NETWORK_STATE_ONLINE;

        if (getEnabled() && isOnline && !wasOnline) {
            startWebServer();
            wasOnline = true;
        } else if ((!getEnabled() || !isOnline) && wasOnline) {
            wasOnline = false;
            stopWebServer();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
