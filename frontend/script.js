// ============================================================
//  Staywise — Hostel Management · script.js
// ============================================================

// ── Edit & Update Resident Details ─────────────────────────
function updateEditSemesterOptions(year, selectedSemester = null) {
    const semSelect = document.getElementById('edit-semester');
    if (!semSelect) return;
    semSelect.innerHTML = '';
    
    const y = parseInt(year) || 1;
    const startSem = (y - 1) * 2 + 1;
    const endSem = startSem + 1;
    
    for (let s = startSem; s <= endSem; s++) {
        const opt = document.createElement('option');
        opt.value = s;
        opt.textContent = `Semester ${s}`;
        if (selectedSemester && s === parseInt(selectedSemester)) {
            opt.selected = true;
        }
        semSelect.appendChild(opt);
    }
}

function openEditModal(id) {
    const s = students.find(x => x.id === id);
    if (!s) return;

    document.getElementById('edit-student-id').value = s.id;
    document.getElementById('edit-modal-title').textContent = `Edit Resident Details — ${s.name} (${s.id})`;
    document.getElementById('edit-name').value = s.name;
    document.getElementById('edit-cgpa').value = s.cgpa;
    document.getElementById('edit-year').value = s.year;
    
    // Dynamic semester coupling based on year of study
    updateEditSemesterOptions(s.year, s.semester);
    
    document.getElementById('edit-room-pref').value = s.roomPref || 2;
    document.getElementById('edit-food-pref').value = s.foodPreference || 1;
    document.getElementById('edit-wants-ac').checked = s.wantsAC || false;
    document.getElementById('edit-wants-mess').checked = s.wantsMess || false;

    document.getElementById('edit-habit-sleep').value = s.sleepHabit || 5;
    document.getElementById('edit-habit-cleanliness').value = s.cleanlinessHabit || 5;
    document.getElementById('edit-habit-study').value = s.studyHabit || 5;

    document.getElementById('edit-final-sem-cgpa').value = s.finalSemesterCGPA || 0.0;
    document.getElementById('edit-exam-passed').checked = s.examPassed || false;
    document.getElementById('edit-has-backlog').checked = s.hasBacklog || false;
    document.getElementById('edit-placement-ext').checked = s.placementExtension || false;

    document.getElementById('edit-modal').style.display = 'flex';
}

function closeEditModal() {
    document.getElementById('edit-modal').style.display = 'none';
}

async function handleEditResidentSubmit(event) {
    event.preventDefault();
    const id = document.getElementById('edit-student-id').value;

    const payload = {
        id: id,
        name: document.getElementById('edit-name').value,
        cgpa: parseFloat(document.getElementById('edit-cgpa').value),
        seniority: parseInt(document.getElementById('edit-year').value),
        semester: parseInt(document.getElementById('edit-semester').value),
        roomPref: parseInt(document.getElementById('edit-room-pref').value),
        foodPreference: parseInt(document.getElementById('edit-food-pref').value),
        wantsAC: document.getElementById('edit-wants-ac').checked,
        wantsMess: document.getElementById('edit-wants-mess').checked,
        sleepHabit: parseInt(document.getElementById('edit-habit-sleep').value),
        cleanlinessHabit: parseInt(document.getElementById('edit-habit-cleanliness').value),
        studyHabit: parseInt(document.getElementById('edit-habit-study').value),
        finalSemCgpa: parseFloat(document.getElementById('edit-final-sem-cgpa').value),
        examPassed: document.getElementById('edit-exam-passed').checked,
        hasBacklog: document.getElementById('edit-has-backlog').checked,
        placementExtension: document.getElementById('edit-placement-ext').checked
    };

    try {
        const res = await fetch('/api/update-student', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (data.success) {
            const promo = data.promotion;
            let msg = `Resident ${data.student.name} updated successfully!`;
            if (promo && promo.message) {
                msg += ` - ${promo.message}`;
            }
            showToast(msg);
            closeEditModal();
            refreshAllData();
        } else {
            showToast(data.error || 'Failed to update resident.', 'error');
        }
    } catch (e) {
        showToast('Error sending update request.', 'error');
    }
}

// ── Delete Resident ──────────────────────────────────────────
let pendingDeleteId = null;

function deleteResidentPrompt(id) {
    const s = students.find(x => x.id === id);
    if (!s) return;

    pendingDeleteId = id;

    const confirmDesc = s.status === 'allocated'
        ? `Warning: Student ${s.name} is currently allocated to room ${s.room}. Deleting them will immediately free their occupancy and remove them from the system permanently.`
        : `Are you sure you want to delete waitlisted student ${s.name} (${s.id}) from the system permanently?`;

    document.getElementById('confirm-modal-desc').textContent = confirmDesc;

    const actionBtn = document.getElementById('confirm-modal-action-btn');
    actionBtn.onclick = async () => {
        actionBtn.disabled = true;
        actionBtn.textContent = 'Deleting...';
        await deleteResident(pendingDeleteId);
        actionBtn.disabled = false;
        actionBtn.textContent = 'Confirm Deletion';
        closeConfirmModal();
    };

    document.getElementById('confirm-modal').style.display = 'flex';
}

function closeConfirmModal() {
    document.getElementById('confirm-modal').style.display = 'none';
    pendingDeleteId = null;
}

async function deleteResident(id) {
    try {
        const res = await fetch(`/api/students?id=${id}`, {
            method: 'DELETE'
        });
        const data = await res.json();
        if (data.success) {
            showToast(`Resident ${id} deleted successfully.`);
            refreshAllData();
        } else {
            showToast(data.error || 'Failed to delete resident.', 'error');
        }
    } catch (e) {
        showToast('Error sending delete request.', 'error');
    }
}

// ── Sample Data ──────────────────────────────────────────────
let rooms = [];
let students = [];
let waitlist = [];


let recentAllocs = [];

// ── Page titles ───────────────────────────────────────────────
const pageTitles = {
    'dashboard': 'Dashboard',
    'rooms': 'Rooms',
    'residents': 'Residents',
    'waitlist': 'Waitlist',
    'assign': 'Assign Rooms',
    'swaps': 'Swap Rooms',
    'compatibility': 'Check Match',
    'add-resident': 'Add Resident',
    'renewal': 'Semester Renewal',
    'academic-status': 'Academic Status',
    'reports': 'Reports',
};

// ── Navigation ────────────────────────────────────────────────
function navigate(page) {
    document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));

    const pageEl = document.getElementById('page-' + page);
    if (pageEl) pageEl.classList.add('active');

    document.querySelectorAll('.nav-item').forEach(btn => {
        const oc = btn.getAttribute('onclick') || '';
        if (oc.includes("'" + page + "'")) btn.classList.add('active');
    });

    document.getElementById('topbar-title').textContent = pageTitles[page] || page;

    if (page === 'rooms') renderRooms();
    if (page === 'residents') renderResidents();
    if (page === 'waitlist') renderWaitlist();
    if (page === 'academic-status') renderYear4Overview();
}

// ── Render helpers ────────────────────────────────────────────
function initials(name) {
    return name.split(' ').map(n => n[0]).join('').toUpperCase();
}

function renderRecentAllocs() {
    const tbody = document.getElementById('recent-alloc-table');
    tbody.innerHTML = recentAllocs.map(s => `
    <tr>
      <td>
        <div style="display:flex;align-items:center;gap:8px">
          <div class="avatar ${s.gender === 'F' ? 'av-blue' : 'av-green'}">${initials(s.name)}</div>
          <span>${s.name}</span>
        </div>
      </td>
      <td><span class="text-mono" style="font-size:13px">${s.room}</span></td>
      <td>${s.status === 'allocated'
            ? '<span class="badge badge-green">Allocated</span>'
            : '<span class="badge badge-amber">Waitlisted</span>'}</td>
    </tr>
  `).join('');
}

function renderWaitlistPreview() {
    const el = document.getElementById('waitlist-preview');
    el.innerHTML = waitlist.slice(0, 4).map(w => `
    <div class="waitlist-item">
      <div class="waitlist-rank">${w.rank}</div>
      <div class="waitlist-info">
        <div class="waitlist-name">${w.name}
          <span style="font-family:var(--font-mono);font-size:11px;color:var(--text-3)">${w.id}</span>
        </div>
        <div class="waitlist-reason">${w.reason}</div>
      </div>
      ${w.special ? '<span class="badge badge-blue">Special</span>' : ''}
    </div>
  `).join('');
}

function renderRooms() {
    const gf = document.getElementById('room-filter-gender').value;
    const tf = document.getElementById('room-filter-type').value;
    const typeNames = ['', 'Single', 'Double', 'Triple', 'Quad', '5-Bed Sharing', '6-Bed Sharing'];

    let filtered = rooms;
    if (gf !== 'all') filtered = filtered.filter(r => r.gender === gf);
    if (tf !== 'all') filtered = filtered.filter(r => r.type == tf);

    const grid = document.getElementById('rooms-grid');
    grid.innerHTML = filtered.map(r => {
        const pct = r.capacity ? r.occupancy / r.capacity : 0;
        const fillClass = pct >= 1 ? 'fill-red' : pct >= 0.6 ? 'fill-amber' : 'fill-green';
        const badgeClass = pct >= 1 ? 'badge-red' : pct === 0 ? 'badge-gray' : 'badge-green';
        const statusText = pct >= 1 ? 'Full' : pct === 0 ? 'Empty' : 'Available';
        const gColor = r.gender === 'M'
            ? 'background:var(--info-light);color:var(--info)'
            : 'background:var(--warn-light);color:var(--warn)';
        return `
      <div class="room-card" onclick="showRoomDetail('${r.id}')">
        <div class="room-card-header">
          <span class="room-id">${r.id}</span>
          <span class="badge ${badgeClass}">${statusText}</span>
        </div>
        <div style="font-size:12px;color:var(--text-2);margin-bottom:6px">
          ${typeNames[r.type] || r.type + '-Bed Sharing'} · ${r.isAC ? 'AC' : 'Non-AC'}
        </div>
        <div class="occupancy-bar">
          <div class="occupancy-fill ${fillClass}" style="width:${pct * 100}%"></div>
        </div>
        <div class="room-meta">
          <span class="room-occupants">${r.occupancy} / ${r.capacity} beds</span>
          <span class="room-gender" style="${gColor}">${r.gender}</span>
        </div>
      </div>
    `;
    }).join('');
}

function renderResidents(filter = '') {
    const riskLabels = ['Low', 'Medium', 'High'];
    const riskBadge = ['badge-green', 'badge-amber', 'badge-red'];
    const tbody = document.getElementById('residents-table');

    const filtered = filter
        ? students.filter(s =>
            s.name.toLowerCase().includes(filter.toLowerCase()) ||
            s.id.toLowerCase().includes(filter.toLowerCase()))
        : students;

    tbody.innerHTML = filtered.map(s => {
        let acadStatusHTML = '';
        if (s.year === 4) {
            if (s.examPassed && !s.hasBacklog && !s.placementExtension) {
                acadStatusHTML = '<span class="badge badge-green" style="font-size:11px">🎓 Grad Eligible</span>';
            } else if (s.placementExtension) {
                acadStatusHTML = '<span class="badge badge-blue" style="font-size:11px">💼 Placement Ext</span>';
            } else {
                let reasons = [];
                if (!s.examPassed) reasons.push('Failed');
                if (s.hasBacklog) reasons.push('Backlog');
                acadStatusHTML = `<span class="badge badge-amber" style="font-size:11px">⏸ Retained (${reasons.join('/')})</span>`;
            }
        } else {
            if (s.examPassed && !s.hasBacklog) {
                acadStatusHTML = '<span class="badge badge-green" style="font-size:11px">✓ Pass / Promoted</span>';
            } else if (s.hasBacklog) {
                acadStatusHTML = '<span class="badge badge-red" style="font-size:11px">⚠ Has Backlogs</span>';
            } else {
                acadStatusHTML = '<span class="badge badge-gray" style="font-size:11px">Pending / Failed</span>';
            }
        }

        const cgpaDisplay = `
            <div style="font-weight:500">${s.cgpa.toFixed(2)}</div>
            ${s.finalSemesterCGPA > 0 ? `<div style="font-size:10px;color:var(--text-3);margin-top:2px">Final Sem: ${s.finalSemesterCGPA.toFixed(2)}</div>` : ''}
        `;

        return `
        <tr>
          <td>
            <div style="display:flex;align-items:center;gap:9px">
              <div class="avatar ${s.gender === 'F' ? 'av-blue' : 'av-green'}">${initials(s.name)}</div>
              <div>
                <div style="font-weight:500;font-size:14px">${s.name}</div>
                ${s.special ? '<span class="badge badge-blue" style="font-size:10px;padding:2px 6px">Special needs</span>' : ''}
              </div>
            </div>
          </td>
          <td><span class="text-mono" style="font-size:12px;color:var(--text-2)">${s.id}</span></td>
          <td>${cgpaDisplay}</td>
          <td>Year ${s.year} <span style="color:var(--text-3);margin:0 4px">·</span> Sem ${s.semester}</td>
          <td><span class="text-mono" style="font-size:12px">${s.room}</span></td>
          <td>${acadStatusHTML}</td>
          <td>${s.status === 'allocated'
                ? '<span class="badge badge-green">Allocated</span>'
                : '<span class="badge badge-amber">Waitlisted</span>'}</td>
          <td style="text-align:right">
            <div style="display:flex; justify-content:flex-end; gap:8px">
              <button class="btn btn-secondary btn-sm" style="padding:4px 10px" onclick="openEditModal('${s.id}')">Edit</button>
              <button class="btn btn-danger btn-sm" style="padding:4px 10px" onclick="deleteResidentPrompt('${s.id}')">Delete</button>
            </div>
          </td>
        </tr>
      `;
    }).join('');

    if (filtered.length === 0) {
        tbody.innerHTML = `
            <tr>
                <td colspan="8" style="text-align:center; padding:32px; color:var(--text-3)">
                    <div style="font-size:16px; margin-bottom:8px">No residents found</div>
                    <div style="font-size:13px">Try matching a different search term or register a new resident.</div>
                </td>
            </tr>
        `;
    }
}

function filterResidents(val) { renderResidents(val); }

function renderWaitlist(filteredList = null) {
    const listToRender = filteredList || waitlist;
    const tbody = document.getElementById('waitlist-table');

    if (listToRender.length === 0) {
        tbody.innerHTML = `
            <tr>
                <td colspan="9" style="text-align:center; padding:32px; color:var(--text-3)">
                    <div style="font-size:16px; margin-bottom:8px">No waitlisted students found</div>
                    <div style="font-size:13px">Try matching a different search term or filter.</div>
                </td>
            </tr>
        `;
        return;
    }

    tbody.innerHTML = listToRender.map(w => {
        const priorityScore = (w.wait * 2.0) + (w.score * 1.5) + (w.rejections * 1.0) + (w.seniority * 0.5) + (w.special ? 50.0 : 0.0) - (w.risk * 5.0);

        let riskBadge = '';
        if (w.risk === 2) {
            riskBadge = '<span class="badge badge-red">High</span>';
        } else if (w.risk === 1) {
            riskBadge = '<span class="badge badge-amber">Medium</span>';
        } else {
            riskBadge = '<span class="badge badge-green">Low</span>';
        }

        return `
        <tr>
          <td><span style="font-weight:600;color:var(--text-2)">#${w.rank}</span></td>
          <td><span style="font-family:var(--font-mono);font-size:13px;font-weight:500">${w.id}</span></td>
          <td>
            <div style="font-weight:500">${w.name}</div>
            ${w.special ? '<span class="badge badge-blue" style="font-size:10px;padding:2px 4px;margin-top:2px">Special Needs</span>' : ''}
          </td>
          <td><span style="color:var(--text-2);font-size:13px">${w.reason}</span></td>
          <td>${w.wait} day${w.wait !== 1 ? 's' : ''}</td>
          <td><span style="font-weight:600;font-family:var(--font-mono)">${w.score}</span></td>
          <td>${riskBadge}</td>
          <td><span style="font-weight:700;color:var(--accent);font-family:var(--font-mono)">${priorityScore.toFixed(1)}</span></td>
          <td>
            <button class="btn btn-secondary btn-sm" style="padding:4px 10px" onclick="showManualAssignModal('${w.id}')">Assign Room</button>
          </td>
        </tr>
      `;
    }).join('');
}

function handleWaitlistSearchFilter() {
    const q = document.getElementById('waitlist-search').value.trim().toLowerCase();
    const gender = document.getElementById('waitlist-filter-gender').value;
    const special = document.getElementById('waitlist-filter-special').value;
    const risk = document.getElementById('waitlist-filter-risk').value;

    const filtered = waitlist.filter(w => {
        if (q && !w.name.toLowerCase().includes(q) && !w.id.toLowerCase().includes(q)) {
            return false;
        }
        if (gender && !w.id.startsWith(`S${gender}`)) {
            return false;
        }
        if (special === 'special' && !w.special) return false;
        if (special === 'standard' && w.special) return false;
        if (risk !== '' && w.risk !== parseInt(risk)) return false;
        return true;
    });

    renderWaitlist(filtered);
}



function showRoomDetail(id) {
    const r = rooms.find(x => x.id === id);
    if (!r) return;
    showToast(`Room ${r.id}: ${r.occupancy}/${r.capacity} occupied`);
}

// ── Actions ───────────────────────────────────────────────────
function parseAndShowPromotions(log) {
    if (!log) return false;

    let found = false;

    // Check waitlist additions (e.g. Added student SM05 (Om) to waitlist)
    const waitlistRegex = /Added student\s+(S[MF]\d+)\s*\(([^)]+)\)\s+to waitlist/gi;
    let wMatch;
    while ((wMatch = waitlistRegex.exec(log)) !== null) {
        showToast('Added to Waitlist', 'warn');
        found = true;
    }

    // Matches expressions like: 
    // - Promoted waitlisted student SM05 (Om) to vacant Room B101
    // - Allocated waitlisted student SM05 (Om) to Room B101
    // - student SM05 (Om) promoted to Room B101
    const promoRegex = /(?:student|Allocated|Promoted)\s+(S[MF]\d+)\s*\(([^)]+)\)\s+(?:promoted to|to vacant Room|to Room)\s+([A-Z0-9-]+)/gi;
    let match;
    while ((match = promoRegex.exec(log)) !== null) {
        const studentId = match[1];
        const roomNo = match[3];
        showToast(`${studentId} promoted to Room ${roomNo}`, 'success');
        found = true;
    }
    return found;
}

// ── Global Operation Lock ─────────────────────────────────────
let _operationInProgress = false;

function setButtonLoading(btn, loading, originalText = '') {
    if (!btn) return;
    if (loading) {
        btn.disabled = true;
        btn._originalHTML = btn.innerHTML;
        btn.innerHTML = `<span class="btn-spinner"></span> Processing…`;
        btn.style.opacity = '0.7';
        btn.style.pointerEvents = 'none';
    } else {
        btn.disabled = false;
        btn.innerHTML = btn._originalHTML || originalText;
        btn.style.opacity = '';
        btn.style.pointerEvents = '';
    }
}

async function runAllocation(triggerBtn = null) {
    if (_operationInProgress) {
        showToast('Another operation is in progress. Please wait.', 'warn');
        return;
    }
    _operationInProgress = true;
    if (triggerBtn) setButtonLoading(triggerBtn, true);
    try {
        const res = await fetch('/api/run-allocation', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            const hasPromotions = parseAndShowPromotions(data.log);

            const allocMatch = data.log.match(/Newly Allocated:\s*(\d+)/);
            const waitMatch = data.log.match(/Newly Waitlisted:\s*(\d+)/);
            const numAlloc = allocMatch ? parseInt(allocMatch[1]) : 0;
            const numWait = waitMatch ? parseInt(waitMatch[1]) : 0;

            if (!hasPromotions) {
                if (numAlloc === 0) {
                    showToast('No rooms available for remaining waitlisted students.', 'warn');
                } else {
                    showToast(`Smart Allocation complete — ${numAlloc} assigned, ${numWait} waitlisted.`);
                }
            }

            refreshAllData();
        }
    } catch (e) {
        showToast('Error running smart allocation engine.', 'error');
    } finally {
        _operationInProgress = false;
        if (triggerBtn) setButtonLoading(triggerBtn, false);
    }
}

async function recoverVacancies(triggerBtn = null, silent = false) {
    if (_operationInProgress) {
        if (!silent) showToast('Another operation is in progress. Please wait.', 'warn');
        return;
    }
    _operationInProgress = true;
    if (triggerBtn) setButtonLoading(triggerBtn, true);
    try {
        const res = await fetch('/api/recover-vacancies', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            const hasPromotions = parseAndShowPromotions(data.log);

            if (!hasPromotions && !silent) {
                if (data.log && data.log.includes('No new high-risk vacancies')) {
                    showToast('All rooms stable — no vacant beds to recover.', 'info');
                } else {
                    const count = (data.log.match(/Vacancy Detected/g) || []).length;
                    showToast(`Vacancy recovery done — ${count} bed(s) reclaimed and reassigned.`);
                }
            }
            refreshAllData();
        }
    } catch (e) {
        if (!silent) showToast('Error running vacancy recovery.', 'error');
    } finally {
        _operationInProgress = false;
        if (triggerBtn) setButtonLoading(triggerBtn, false);
    }
}

async function runRenewal(triggerBtn = null) {
    if (_operationInProgress) {
        showToast('Another operation is in progress. Please wait.', 'warn');
        return;
    }
    _operationInProgress = true;
    if (triggerBtn) setButtonLoading(triggerBtn, true);
    try {
        const res = await fetch('/api/semester-renewal', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            parseAndShowPromotions(data.log);

            if (data.log && data.log.includes("No Year-4 students")) {
                showToast("No Year-4 students found in the system.", "info");
            } else if (data.log && data.log.includes("No eligible graduates")) {
                const retained = (data.log.match(/Retained:/g) || []).length;
                showToast(`No eligible graduates. ${retained} student(s) retained (backlog/failed/extension).`, "warn");
            } else {
                const checkouts = (data.log.match(/Checkout:/g) || []).length;
                const retained = (data.log.match(/Retained:/g) || []).length;
                const promotions = (data.log.match(/Promotion:/g) || []).length;
                showToast(`Semester renewal: ${checkouts} graduated, ${retained} retained, ${promotions} promoted.`);
            }
            refreshAllData();

            // After semester processing, auto-trigger lightweight vacancy recovery
            // This fills only newly vacated beds — NOT a full smart allocation
            _operationInProgress = false;
            showToast('Running post-renewal vacancy recovery…', 'info');
            await recoverVacancies(null, true);
            showToast('Post-renewal vacancy recovery complete.', 'success');
        }
    } catch (e) {
        showToast('Error processing semester renewal.', 'error');
    } finally {
        _operationInProgress = false;
        if (triggerBtn) setButtonLoading(triggerBtn, false);
    }
}

async function graduationCheckout(triggerBtn = null) {
    if (_operationInProgress) {
        showToast('Another operation is in progress. Please wait.', 'warn');
        return;
    }
    _operationInProgress = true;
    if (triggerBtn) setButtonLoading(triggerBtn, true);
    try {
        const res = await fetch('/api/graduation-checkout', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            if (data.graduated === 0) {
                const retained = (data.skippedFailed || 0) + (data.skippedBacklog || 0) + (data.skippedExtension || 0) + (data.skippedSemester || 0);
                if (retained > 0) {
                    showToast(`No eligible graduates found. ${retained} student(s) retained (backlog/failed/extension/semester pending).`, 'warn');
                } else {
                    showToast('No students eligible for graduation checkout.', 'info');
                }
            } else {
                let msg = `${data.graduated} student(s) graduated and checked out.`;
                if (data.promoted > 0) {
                    msg += ` ${data.promoted} waitlisted student(s) promoted to vacated rooms.`;
                }
                showToast(msg, 'success');
            }
            refreshAllData();
        } else {
            showToast(data.error || 'Graduation checkout failed.', 'error');
        }
    } catch (e) {
        showToast('Error processing graduation checkout.', 'error');
    } finally {
        _operationInProgress = false;
        if (triggerBtn) setButtonLoading(triggerBtn, false);
    }
}

async function addSwapLink() {
    const id1 = document.getElementById('swap-id1').value.trim().toUpperCase();
    const id2 = document.getElementById('swap-id2').value.trim().toUpperCase();
    if (!id1 || !id2) { showToast('Please enter both resident IDs.', 'warn'); return; }

    // Quick frontend check using the ID format (SM01 vs SF14)
    if (id1[1] !== id2[1]) {
        showToast('Cross-gender swaps are not permitted.', 'error');
        return;
    }

    try {
        const res = await fetch('/api/add-swap', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id1, id2 })
        });
        const data = await res.json();
        if (data.success) {
            showToast(`Swap link added between ${id1} and ${id2}`);
            document.getElementById('swap-id1').value = '';
            document.getElementById('swap-id2').value = '';
        } else {
            showToast(data.error || 'Failed to add swap', 'error');
        }
    } catch (e) {
        showToast('Error adding swap', 'error');
    }
}

async function previewSwap() {
    await previewSwapChain();
}

async function previewSwapChain() {
    try {
        const startIdInput = document.getElementById('swapStartId') || document.getElementById('swap-start');
        const startId = startIdInput ? startIdInput.value.trim() : '';
        if (!startId) {
            showToast('Enter starting resident ID', 'error');
            return;
        }
        const response = await fetch(
            `/api/preview-swap?startId=${encodeURIComponent(startId)}`
        );
        if (!response.ok) {
            throw new Error('Server error');
        }
        const data = await response.json();
        const container =
            document.getElementById('chainPreview');
        container.innerHTML = '';
        if (!data.chain || data.chain.length === 0) {
            container.innerHTML =
                '<p>No swap chain found</p>';
            return;
        }
        data.chain.forEach((id, index) => {
            const node = document.createElement('div');
            node.className = 'chain-node';
            node.innerText = id;
            container.appendChild(node);
            if (index < data.chain.length - 1) {
                const arrow =
                    document.createElement('span');
                arrow.className = 'chain-arrow';
                arrow.innerHTML = '→';
                container.appendChild(arrow);
            }
        });
    } catch (error) {
        console.error(error);
        showToast(
            'Error loading swap preview',
            'error'
        );
    }
}

async function processSwap() {
    const startIdInput = document.getElementById('swapStartId') || document.getElementById('swap-start');
    const startId = startIdInput ? startIdInput.value.trim() : '';
    if (!startId) { showToast('Enter a starting resident ID.', 'warn'); return; }

    try {
        const res = await fetch('/api/process-swap', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ startId })
        });
        const data = await res.json();
        if (data.success) {
            showToast('Room swap chain executed successfully.');
            if (startIdInput) startIdInput.value = '';
            const preview = document.getElementById('swap-preview');
            if (preview) preview.style.display = 'none';
            refreshAllData();
        } else {
            showToast(data.error || 'Swap execution failed', 'error');
        }
    } catch (e) {
        showToast('Error executing swap', 'error');
    }
}

async function undoSwap() {
    try {
        const res = await fetch('/api/undo-swap', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            showToast('Last swap has been undone.');
            refreshAllData();
        } else {
            showToast(data.error || 'No swaps to undo', 'warn');
        }
    } catch (e) {
        showToast('Error undoing swap', 'error');
    }
}

async function checkCompatibility() {
    const id1 = document.getElementById('compat-id1').value.trim();
    const id2 = document.getElementById('compat-id2').value.trim();
    if (!id1 || !id2) { showToast('Please enter both resident IDs.', 'warn'); return; }

    try {
        const res = await fetch('/api/compatibility', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id1, id2 })
        });
        const data = await res.json();
        if (!res.ok) {
            showToast(data.error || 'Compatibility check failed', 'error');
            return;
        }

        const score = data.score;
        const isSafe = score >= 40;
        const resultEl = document.getElementById('compat-result');

        resultEl.innerHTML = `
        <div style="text-align:center;padding:8px 0">
          <div style="font-size:40px;font-weight:700;letter-spacing:-0.04em;color:${isSafe ? 'var(--accent)' : 'var(--danger)'}">
            ${score}%
          </div>
          <div style="font-size:13px;color:var(--text-2);margin:4px 0 16px">${data.name1} × ${data.name2}</div>
          <div class="compat-meter">
            <div class="compat-fill" style="width:${score}%;background:${isSafe ? 'var(--accent)' : '#dc2626'}"></div>
          </div>
          <div class="badge ${isSafe ? 'badge-green' : 'badge-red'}" style="display:inline-flex;margin-top:8px">
            ${isSafe ? '✓ Compatible — safe to room together' : '✗ Risky — high conflict probability'}
          </div>
        </div>
        `;
    } catch (e) {
        showToast('Error calculating compatibility', 'error');
    }
}

async function submitNewResident(triggerBtn = null) {
    const id = document.getElementById('new-id').value.trim();
    const name = document.getElementById('new-name').value.trim();
    if (!id || !name) { showToast('Please fill in all required fields.', 'warn'); return; }

    const gender = document.getElementById('new-gender').value;
    if (gender === 'M' && !id.startsWith('SM')) { showToast('Male IDs must start with SM.', 'warn'); return; }
    if (gender === 'F' && !id.startsWith('SF')) { showToast('Female IDs must start with SF.', 'warn'); return; }

    const roomPref = parseInt(document.getElementById('new-room-pref').value);
    const newStudent = {
        id, name, gender,
        cgpa: parseFloat(document.getElementById('new-cgpa').value) || 0,
        seniority: parseInt(document.getElementById('new-year').value),
        specialNeeds: document.getElementById('new-special').value === '1',
        roomPref: roomPref,
        wantsAC: document.getElementById('new-ac').value === '1',
        wantsMess: document.getElementById('new-mess').value === '1',
        foodPreference: parseInt(document.getElementById('new-food').value),
        distance: parseInt(document.getElementById('new-distance').value) || 0,
        prevPattern: document.getElementById('new-prevpattern').value === '1',
        sleepHabit: parseInt(document.getElementById('new-sleep').value) || 5,
        cleanlinessHabit: parseInt(document.getElementById('new-clean').value) || 5,
        studyHabit: parseInt(document.getElementById('new-study').value) || 5
    };

    if (triggerBtn) setButtonLoading(triggerBtn, true);
    try {
        // Step 1: Register the student (single student registration only)
        const res = await fetch('/api/students', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(newStudent)
        });
        const data = await res.json();

        if (res.ok && data.success) {
            // Step 2: Check if exact preferred room type is available
            // Only allocate immediately if the preferred room exists and has space
            const prefGender = gender;
            const prefType = roomPref;
            const vacantPreferred = rooms.filter(r => 
                r.gender === prefGender && 
                r.type === prefType && 
                r.occupancy < r.capacity
            );

            if (vacantPreferred.length > 0) {
                // Preferred room available → try immediate allocation via manual-assign to first vacant
                try {
                    const assignRes = await fetch('/api/manual-assign', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ studentId: id, roomId: vacantPreferred[0].id })
                    });
                    const assignData = await assignRes.json();
                    if (assignData.success) {
                        showToast(`${name} (${id}) registered and allocated to Room ${vacantPreferred[0].id}.`);
                    } else {
                        // Compatibility check failed or room filled — student goes to waitlist
                        showToast(`${name} (${id}) registered. Preferred room unavailable (compatibility mismatch) — added to waitlist.`, 'warn');
                    }
                } catch {
                    showToast(`${name} (${id}) registered. Could not auto-assign preferred room — added to waitlist.`, 'warn');
                }
            } else {
                // No preferred room available → student stays on waitlist
                showToast(`${name} (${id}) registered. No ${prefType}-sharing rooms available — added to waitlist.`, 'warn');
            }

            // NOTE: We do NOT run full smart allocation, vacancy recovery, or hostel reshuffling here.
            // The student is either allocated to their exact preferred room or waitlisted.

            refreshAllData();
            navigate('residents');

            // Clear the registration form
            document.getElementById('new-id').value = '';
            document.getElementById('new-name').value = '';
            document.getElementById('new-cgpa').value = '';
            document.getElementById('new-distance').value = '';
            document.getElementById('new-gender').value = 'M';
            document.getElementById('new-year').value = '1';
            document.getElementById('new-special').value = '0';
            document.getElementById('new-room-pref').value = '1';
            document.getElementById('new-ac').value = '0';
            document.getElementById('new-mess').value = '0';
            document.getElementById('new-food').value = '1';
            document.getElementById('new-prevpattern').value = '0';
            document.getElementById('new-sleep').value = '5';
            document.getElementById('new-clean').value = '5';
            document.getElementById('new-study').value = '5';
            updateIDPrefix();
        } else {
            showToast(data.error || 'Failed to register student.', 'error');
        }
    } catch (e) {
        showToast('Error registering student.', 'error');
    } finally {
        if (triggerBtn) setButtonLoading(triggerBtn, false);
    }
}

function updateIDPrefix() {
    const gender = document.getElementById('new-gender').value;
    document.getElementById('id-hint').textContent =
        gender === 'M' ? 'Use format: SM## for male' : 'Use format: SF## for female';
}

// ── Academic Status ──────────────────────────────────────────
let _acadLoadedStudent = null;

function semesterLabel(year, semester) {
    return `Year ${year} · Semester ${semester}`;
}

function promotionStatusHTML(student) {
    const eligible = student.examPassed && !student.hasBacklog;
    if (student.seniority === 4) {
        if (eligible && !student.placementExtension) {
            return '<span class="badge badge-green">Eligible for Graduation</span>';
        } else if (eligible && student.placementExtension) {
            return '<span class="badge badge-amber">Retained — Placement Extension</span>';
        }
    }
    if (eligible && student.seniority < 4) {
        return `<span class="badge badge-green">Eligible for Promotion → Year ${student.seniority + 1}</span>`;
    }
    let reasons = [];
    if (!student.examPassed) reasons.push('Exam not passed');
    if (student.hasBacklog) reasons.push('Has backlog');
    return `<span class="badge badge-amber">Not Eligible${reasons.length ? ' — ' + reasons.join(', ') : ''}</span>`;
}

async function loadAcademicStatus() {
    const sid = document.getElementById('acad-student-id').value.trim().toUpperCase();
    if (!sid) { showToast('Please enter a Student ID.', 'warn'); return; }

    try {
        const res = await fetch('/api/students');
        if (!res.ok) throw new Error();
        const data = await res.json();
        const student = data.find(s => s.id === sid);
        if (!student) {
            showToast(`Student ${sid} not found.`, 'error');
            document.getElementById('acad-form-fields').style.display = 'none';
            document.getElementById('acad-promotion-result').style.display = 'none';
            return;
        }
        _acadLoadedStudent = student;
        document.getElementById('acad-form-fields').style.display = 'block';
        document.getElementById('acad-promotion-result').style.display = 'none';

        // Dynamic academic status variables
        const isEligibleForGraduation = student.seniority === 4 && student.examPassed && !student.hasBacklog && !student.placementExtension;
        const gradStatusHTML = isEligibleForGraduation
            ? '<span class="badge badge-green">✓ Eligible for Graduation</span>'
            : student.seniority === 4
                ? '<span class="badge badge-amber">✗ Retained — Ineligible for Graduation</span>'
                : '<span class="badge badge-gray">N/A — Continuing Student</span>';

        const backlogHTML = student.hasBacklog
            ? '<span class="badge badge-red">⚠ Active Backlogs</span>'
            : '<span class="badge badge-green">✓ No Backlogs (Clear)</span>';

        const historyHTML = `
            <div style="margin-top:16px;padding:16px;background:var(--surface2);border-radius:var(--radius);border:1px solid var(--border)">
                <div style="font-weight:600;font-size:12px;text-transform:uppercase;letter-spacing:0.05em;color:var(--text-2);margin-bottom:12px;border-bottom:1px solid var(--border);padding-bottom:6px">
                    Academic Progress & History
                </div>
                <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;font-size:13px">
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Current Academic Year</div>
                        <div style="font-weight:500;margin-top:2px">Year ${student.seniority}</div>
                    </div>
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Current Semester</div>
                        <div style="font-weight:500;margin-top:2px">Semester ${student.semester}</div>
                    </div>
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Overall CGPA</div>
                        <div style="font-weight:600;font-family:var(--font-mono);margin-top:2px;color:var(--accent)">${student.cgpa.toFixed(2)}</div>
                    </div>
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Final Semester CGPA</div>
                        <div style="font-weight:600;font-family:var(--font-mono);margin-top:2px">${student.finalSemesterCGPA > 0 ? student.finalSemesterCGPA.toFixed(2) : '—'}</div>
                    </div>
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Exam Passed Status</div>
                        <div style="font-weight:500;margin-top:2px">${student.examPassed ? '✓ Cleared' : '✗ Pending / Failed'}</div>
                    </div>
                    <div>
                        <div style="color:var(--text-3);font-size:11px">Backlog Status</div>
                        <div style="font-weight:500;margin-top:2px">${backlogHTML}</div>
                    </div>
                    <div style="grid-column:span 2">
                        <div style="color:var(--text-3);font-size:11px">Graduation Eligibility</div>
                        <div style="font-weight:500;margin-top:2px">${gradStatusHTML}</div>
                    </div>
                </div>
            </div>
        `;

        document.getElementById('acad-student-info').innerHTML = `
            <div style="font-weight:600;font-size:16px">${student.name} <span class="text-mono" style="font-size:12px;color:var(--text-3)">(${student.id})</span></div>
            <div class="text-sm text-muted" style="margin-top:4px;display:flex;gap:8px;align-items:center;flex-wrap:wrap">
                <span>${student.gender === 'M' ? 'Male' : 'Female'}</span> · 
                <span>${student.isAllocated ? 'Room ' + student.currentRoom : 'Waitlisted'}</span>
            </div>
            <div style="margin-top:12px;display:flex;gap:8px;flex-wrap:wrap">
                ${promotionStatusHTML(student)}
            </div>
            ${historyHTML}
        `;

        // Populate form fields
        document.getElementById('acad-current-year').textContent = `Year ${student.seniority}`;
        document.getElementById('acad-current-sem').textContent = `Semester ${student.semester}`;
        document.getElementById('acad-updated-cgpa').value = student.cgpa || '';
        document.getElementById('acad-final-cgpa').value = student.finalSemesterCGPA || '';
        document.getElementById('acad-exam-passed').value = student.examPassed ? '1' : '0';
        document.getElementById('acad-has-backlog').value = student.hasBacklog ? '1' : '0';
        document.getElementById('acad-placement-ext').value = student.placementExtension ? '1' : '0';

        // Show/hide placement extension field (only relevant for Year 4)
        document.getElementById('acad-placement-group').style.display =
            student.seniority === 4 ? 'block' : 'none';
    } catch (e) {
        showToast('Error loading student data', 'error');
    }
}

async function saveAcademicStatus() {
    if (!_acadLoadedStudent) { showToast('No student loaded.', 'warn'); return; }

    const studentName = _acadLoadedStudent.name;
    const payload = {
        id: _acadLoadedStudent.id,
        updatedCgpa: document.getElementById('acad-updated-cgpa').value || '',
        finalSemCgpa: document.getElementById('acad-final-cgpa').value || '0',
        examPassed: document.getElementById('acad-exam-passed').value,
        hasBacklog: document.getElementById('acad-has-backlog').value,
        placementExtension: document.getElementById('acad-placement-ext').value
    };

    try {
        const res = await fetch('/api/update-student', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        if (!res.ok) {
            const errText = await res.text();
            showToast(errText || 'Server error while saving', 'error');
            return;
        }
        const data = await res.json();
        if (data.success) {
            // Show promotion result
            const promo = data.promotion;
            if (promo.promoted) {
                showToast(`✓ ${studentName}: ${promo.message}`);
            } else if (promo.graduationEligible) {
                showToast(`🎓 ${studentName}: ${promo.message}`);
            } else {
                showToast(`${studentName}: ${promo.message}`, 'warn');
            }

            // Show promotion result card
            const resultEl = document.getElementById('acad-promotion-result');
            const isPositive = promo.promoted || promo.graduationEligible;
            resultEl.innerHTML = `
                <div style="padding:16px;background:${isPositive ? 'var(--accent-light, rgba(22,163,74,0.08))' : 'var(--warn-light, rgba(217,119,6,0.08))'};border-radius:var(--radius);border-left:3px solid ${isPositive ? 'var(--accent)' : 'var(--warn)'}">
                    <div style="font-weight:600;font-size:14px;margin-bottom:4px">
                        ${promo.promoted ? '↑ Year Promoted' : promo.graduationEligible ? '🎓 Graduation Eligible' : '⏸ No Promotion'}
                    </div>
                    <div class="text-sm text-muted">${promo.message}</div>
                    ${promo.promoted ? `<div class="text-xs text-muted" style="margin-top:6px">Year ${promo.previousYear} → Year ${promo.newYear} · Semester ${promo.previousSemester} → Semester ${promo.newSemester}</div>` : ''}
                </div>
            `;
            resultEl.style.display = 'block';

            // Reset form fields
            _acadLoadedStudent = null;
            document.getElementById('acad-student-id').value = '';
            document.getElementById('acad-form-fields').style.display = 'none';
            document.getElementById('acad-student-info').innerHTML = '';
            document.getElementById('acad-updated-cgpa').value = '';
            document.getElementById('acad-final-cgpa').value = '';
            document.getElementById('acad-exam-passed').value = '0';
            document.getElementById('acad-has-backlog').value = '0';
            document.getElementById('acad-placement-ext').value = '0';

            refreshAllData();
            renderYear4Overview();
        } else {
            showToast(data.error || 'Failed to update student record', 'error');
        }
    } catch (e) {
        showToast('Error saving student record: ' + e.message, 'error');
    }
}

async function renderYear4Overview() {
    try {
        const res = await fetch('/api/students');
        if (!res.ok) return;
        const data = await res.json();
        const year4 = data.filter(s => s.seniority === 4);
        const container = document.getElementById('acad-year4-list');
        if (!container) return;

        if (year4.length === 0) {
            container.innerHTML = '<div class="text-sm text-muted">No Year-4 students in the system.</div>';
            return;
        }

        container.innerHTML = year4.map(s => {
            const eligible = s.examPassed && !s.hasBacklog && !s.placementExtension;
            const statusBadge = eligible
                ? '<span class="badge badge-green">Eligible for Checkout</span>'
                : '<span class="badge badge-amber">Retained</span>';

            let reasons = [];
            if (!s.examPassed) reasons.push('Exam not passed');
            if (s.hasBacklog) reasons.push('Has backlog');
            if (s.placementExtension) reasons.push('Placement extension');
            const reasonText = reasons.length > 0 ? reasons.join(', ') : 'All clear';

            return `
                <div style="display:flex;justify-content:space-between;align-items:center;padding:12px;background:var(--surface2);border-radius:var(--radius)">
                    <div>
                        <div style="font-weight:600;font-size:14px">${s.name} <span class="text-muted text-xs">${s.id}</span></div>
                        <div class="text-xs text-muted" style="margin-top:2px">Sem ${s.semester} · ${reasonText} · Final CGPA: ${s.finalSemesterCGPA || 'N/A'}</div>
                    </div>
                    <div>${statusBadge}</div>
                </div>
            `;
        }).join('');
    } catch (e) {
        console.error('Failed to render Year-4 overview', e);
    }
}

// ── Reports ───────────────────────────────────────────────────
async function downloadReport(type) {
    try {
        const response = await fetch('/api/reports?file=' + type);
        if (!response.ok) throw new Error('Network response was not ok');
        const blob = await response.blob();
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.style.display = 'none';
        a.href = url;
        a.download = type + '.txt';
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
        showToast(`${type.replace(/_/g, ' ')} exported.`);
    } catch (e) {
        showToast('Failed to export report', 'error');
    }
}

function exportAllReports() {
    ['room_allocations', 'waitlist_report', 'student_records', 'analytics_report']
        .forEach((r, i) => setTimeout(() => downloadReport(r), i * 200));
}

// ── Toast ─────────────────────────────────────────────────────
function showToast(msg, type = 'success') {
    const container = document.getElementById('toasts');
    const toast = document.createElement('div');
    toast.className = 'toast toast-enter';
    const colors = { success: '#1a1916', warn: '#92400e', error: '#7f1d1d', info: '#1e3a5f' };
    toast.style.background = colors[type] || colors.success;
    const icons = { success: '✓', warn: '⚠', error: '✕', info: 'ℹ' };
    const icon = icons[type] || icons.success;
    toast.innerHTML = `<span>${icon}</span><span>${msg}</span>`;
    container.appendChild(toast);

    // Trigger entrance animation
    requestAnimationFrame(() => toast.classList.add('toast-visible'));

    setTimeout(() => {
        toast.classList.remove('toast-visible');
        toast.classList.add('toast-exit');
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}

// ── Init & Fetch Data ─────────────────────────────────────────
async function fetchDashboardStats() {
    try {
        const res = await fetch('/api/status');
        if (!res.ok) return;
        const data = await res.json();

        document.getElementById('dashboard-occupancy').innerHTML = `${Math.round(data.occupancyRate)}<span style="font-size:16px;color:var(--text-2)">%</span>`;
        document.getElementById('dashboard-beds-filled').textContent = `${data.occupancy} / ${data.totalCapacity} beds filled`;

        // Total residents is set after students are fetched
        document.getElementById('dashboard-total-rooms').textContent = `Across ${data.totalRooms} rooms`;

        document.getElementById('dashboard-waitlisted').textContent = data.waitlistSize;

        document.getElementById('dashboard-avg-match').innerHTML = `${Math.round(data.avgCompatibility)}<span style="font-size:16px;color:var(--text-2)">%</span>`;

        const availableBeds = data.totalCapacity - data.occupancy;
        document.getElementById('dashboard-rooms-available').textContent = availableBeds;

        // Assign Rooms section
        document.getElementById('assign-allocated').textContent = data.totalAllocations;
        document.getElementById('assign-waitlisted').textContent = data.waitlistSize;
        document.getElementById('assign-open-beds').textContent = availableBeds;

        // Reports section
        document.getElementById('report-total-allocations').textContent = data.totalAllocations;
        document.getElementById('report-failed-allocations').textContent = data.failedAllocations;
        document.getElementById('report-successful-swaps').textContent = data.successfulSwaps;
        document.getElementById('report-avg-compatibility').innerHTML = `${Math.round(data.avgCompatibility)}<span style="font-size:14px">%</span>`;

    } catch (e) {
        console.error("Failed to fetch dashboard stats", e);
    }
}

async function fetchRoomsData() {
    try {
        const res = await fetch('/api/rooms');
        if (!res.ok) return;
        const data = await res.json();
        rooms = data;

        // Calculate room utilization by type
        let singleCap = 0, singleOcc = 0, doubleCap = 0, doubleOcc = 0, tripleCap = 0, tripleOcc = 0, quadCap = 0, quadOcc = 0;
        rooms.forEach(r => {
            if (r.type === 1) { singleCap += r.capacity; singleOcc += r.occupancy; }
            if (r.type === 2) { doubleCap += r.capacity; doubleOcc += r.occupancy; }
            if (r.type === 3) { tripleCap += r.capacity; tripleOcc += r.occupancy; }
            if (r.type === 4) { quadCap += r.capacity; quadOcc += r.occupancy; }
        });

        const singlePct = singleCap ? Math.round((singleOcc / singleCap) * 100) : 0;
        const doublePct = doubleCap ? Math.round((doubleOcc / doubleCap) * 100) : 0;
        const triplePct = tripleCap ? Math.round((tripleOcc / tripleCap) * 100) : 0;
        const quadPct = quadCap ? Math.round((quadOcc / quadCap) * 100) : 0;

        document.getElementById('util-single-fill').style.width = singlePct + '%';
        document.getElementById('util-single-val').textContent = singlePct + '%';
        document.getElementById('util-double-fill').style.width = doublePct + '%';
        document.getElementById('util-double-val').textContent = doublePct + '%';
        document.getElementById('util-triple-fill').style.width = triplePct + '%';
        document.getElementById('util-triple-val').textContent = triplePct + '%';
        document.getElementById('util-quad-fill').style.width = quadPct + '%';
        document.getElementById('util-quad-val').textContent = quadPct + '%';

        if (document.getElementById('page-rooms').classList.contains('active')) {
            renderRooms();
        }
    } catch (e) {
        console.error("Failed to fetch rooms data", e);
    }
}

async function fetchStudentsData() {
    try {
        const res = await fetch('/api/students');
        if (!res.ok) return;
        const data = await res.json();
        students = data.map(s => ({
            id: s.id,
            name: s.name,
            gender: s.gender,
            cgpa: s.cgpa,
            year: s.seniority,
            semester: s.semester,
            room: s.isAllocated ? s.currentRoom : '-',
            status: s.isAllocated ? 'allocated' : 'waitlisted',
            risk: s.riskLevel,
            allocationOrder: s.allocationOrder || 0,
            special: s.specialNeeds,
            examPassed: s.examPassed,
            hasBacklog: s.hasBacklog,
            placementExtension: s.placementExtension,
            finalSemesterCGPA: s.finalSemesterCGPA,
            roomPref: s.roomPref,
            wantsAC: s.wantsAC,
            wantsMess: s.wantsMess,
            foodPreference: s.foodPreference,
            sleepHabit: s.sleepHabit,
            cleanlinessHabit: s.cleanlinessHabit,
            studyHabit: s.studyHabit
        }));

        // Populate recentAllocs from students sorted by allocationOrder descending
        recentAllocs = students
            .filter(s => s.status === 'allocated')
            .sort((a, b) => b.allocationOrder - a.allocationOrder)
            .slice(0, 5);

        // Update dashboard total residents to show all registered students
        document.getElementById('dashboard-total-residents').textContent = students.length;

        // Calculate semester renewal stats with academic eligibility
        // Only count Year-4 students as "eligible" if examPassed && !hasBacklog && !placementExtension
        const year4Students = students.filter(s => s.year === 4);
        const eligibleCount = year4Students.filter(s => s.examPassed && !s.hasBacklog && !s.placementExtension).length;
        const waitlistCount = students.filter(s => s.status === 'waitlisted').length;

        if (document.getElementById('renewal-graduating-count')) {
            document.getElementById('renewal-graduating-count').textContent = eligibleCount;
            document.getElementById('renewal-rooms-freed').textContent = eligibleCount;
            document.getElementById('renewal-waitlisted-promote').textContent = waitlistCount;
        }

        renderRecentAllocs();

        if (document.getElementById('page-residents').classList.contains('active')) {
            renderResidents();
        }
    } catch (e) {
        console.error("Failed to fetch students data", e);
    }
}

async function fetchWaitlistData() {
    try {
        const res = await fetch('/api/waitlist');
        if (!res.ok) return;
        const data = await res.json();
        waitlist = data.map((w, index) => ({
            rank: index + 1,
            id: w.id,
            name: w.name,
            wait: w.waitTime || 0,
            score: w.fairnessScore || 0,
            reason: w.waitReason || 'No Rooms Available',
            special: w.specialNeeds || false,
            rejections: w.rejections || 0,
            seniority: w.seniority || 1,
            risk: w.riskLevel || 0
        }));

        document.getElementById('waitlist-count-badge').textContent = `${waitlist.length} waiting`;
        renderWaitlistPreview();

        // Update Waitlist Analytics Cards
        const totalCount = waitlist.length;
        const specialCount = waitlist.filter(w => w.special).length;
        const highRiskCount = waitlist.filter(w => w.risk === 2).length;
        const avgWait = totalCount > 0 ? (waitlist.reduce((acc, w) => acc + w.wait, 0) / totalCount).toFixed(1) : 0;

        const totalEl = document.getElementById('waitlist-analytics-total');
        if (totalEl) totalEl.innerHTML = `${totalCount}<span style="font-size:14px;color:var(--text-3)"> students</span>`;

        const specialEl = document.getElementById('waitlist-analytics-special');
        if (specialEl) specialEl.innerHTML = `${specialCount}<span style="font-size:14px;color:var(--text-3)"> priority</span>`;

        const riskEl = document.getElementById('waitlist-analytics-risk');
        if (riskEl) riskEl.innerHTML = `${highRiskCount}<span style="font-size:14px;color:var(--text-3)"> flagged</span>`;

        const timeEl = document.getElementById('waitlist-analytics-time');
        if (timeEl) timeEl.innerHTML = `${avgWait}<span style="font-size:14px;color:var(--text-3)"> days avg</span>`;

        if (document.getElementById('page-waitlist').classList.contains('active')) {
            renderWaitlist();
        }
    } catch (e) {
        console.error("Failed to fetch waitlist data", e);
    }
}

function refreshAllData() {
    fetchDashboardStats();
    fetchRoomsData();
    fetchStudentsData();
    fetchWaitlistData();
    fetchSettings();
}

function init() {
    const now = new Date();
    document.getElementById('topbar-date').textContent =
        now.toLocaleDateString('en-IN', { weekday: 'short', day: 'numeric', month: 'short', year: 'numeric' });

    document.getElementById('room-filter-gender').addEventListener('change', renderRooms);
    document.getElementById('room-filter-type').addEventListener('change', renderRooms);
    
    const editYear = document.getElementById('edit-year');
    if (editYear) {
        editYear.addEventListener('change', (e) => {
            updateEditSemesterOptions(e.target.value);
        });
    }

    // Fetch real data on init
    refreshAllData();
}

function showManualAssignModal(studentId) {
    const student = waitlist.find(w => w.id === studentId);
    if (!student) return;
    
    const gender = student.id.startsWith('SF') ? 'F' : 'M';
    
    document.getElementById('manual-assign-student-name').textContent = `${student.name} (${student.id})`;
    document.getElementById('manual-assign-student-id').value = student.id;
    
    const roomSelect = document.getElementById('manual-assign-room-select');
    roomSelect.innerHTML = '';
    
    const vacantRooms = rooms.filter(r => r.gender === gender && r.occupancy < r.capacity);
    
    if (vacantRooms.length === 0) {
        roomSelect.innerHTML = `<option value="">No vacant ${gender === 'M' ? 'Boys' : 'Girls'} rooms available</option>`;
    } else {
        vacantRooms.forEach(r => {
            const spaces = r.capacity - r.occupancy;
            const opt = document.createElement('option');
            opt.value = r.id;
            opt.textContent = `Room ${r.id} (${r.isAC ? 'AC' : 'Non-AC'}, Type: ${r.type} Sharing, ${spaces} space${spaces !== 1 ? 's' : ''} left)`;
            roomSelect.appendChild(opt);
        });
    }
    
    document.getElementById('manual-assign-modal').style.display = 'flex';
}

function closeManualAssignModal() {
    document.getElementById('manual-assign-modal').style.display = 'none';
}

async function submitManualAssign() {
    const studentId = document.getElementById('manual-assign-student-id').value;
    const roomId = document.getElementById('manual-assign-room-select').value;
    const btn = document.querySelector('#manual-assign-modal .btn-accent');
    
    if (!roomId) {
        showToast('Please select a valid vacant room.', 'warn');
        return;
    }
    
    // Manual assign: ONLY directly allocate the selected student to the chosen room.
    // Does NOT trigger smart allocation, vacancy recovery, or hostel reshuffling.
    if (btn) setButtonLoading(btn, true);
    try {
        const res = await fetch('/api/manual-assign', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ studentId, roomId })
        });
        const data = await res.json();
        
        if (data.success) {
            closeManualAssignModal();
            showToast(`${studentId} manually assigned to Room ${roomId}.`, 'success');
            refreshAllData();
        } else {
            showToast(data.error || 'Manual assignment failed — check compatibility.', 'error');
        }
    } catch (e) {
        showToast('Error sending manual assignment request.', 'error');
    } finally {
        if (btn) setButtonLoading(btn, false);
    }
}

async function fetchSettings() {
    try {
        const res = await fetch('/api/settings');
        const data = await res.json();
        const checkbox = document.getElementById('setting-allow-upgrade');
        if (checkbox) {
            checkbox.checked = data.allowUpgrade || false;
        }
    } catch (e) {
        console.error("Failed to fetch settings", e);
    }
}

async function toggleAllowUpgradeSetting(checked) {
    try {
        const res = await fetch('/api/settings', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ allowUpgrade: checked })
        });
        const data = await res.json();
        if (data.success) {
            showToast(`Room upgrade preference ${checked ? 'enabled' : 'disabled'} successfully.`, 'success');
            refreshAllData();
        } else {
            showToast('Failed to update room upgrade setting.', 'error');
        }
    } catch (e) {
        showToast('Error updating room upgrade setting.', 'error');
    }
}

init();