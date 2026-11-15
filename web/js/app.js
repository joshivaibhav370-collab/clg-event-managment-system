// app.js — CLG Event Management System
// Shared functions used by the pages that talk to the C++ backend.

// Highlight the active sidebar link based on current page
(function () {
    const currentPage = window.location.pathname.split("/").pop() || "index.html";
    document.querySelectorAll(".sidebar nav ul li a").forEach(link => {
        link.classList.toggle("active", link.getAttribute("href") === currentPage);
    });
})();

// Call the C++ server.  Example: api("register", { student: "S101", event: "E101" })
async function api(path, params) {
    const query = params ? "?" + new URLSearchParams(params).toString() : "";
    try {
        const response = await fetch("/api/" + path + query);
        return await response.json();
    } catch (err) {
        showAlert("Cannot reach the C++ server. Start it with run.bat and open http://localhost:8080", "danger");
        return null;
    }
}

// Show a message box at the top of the page
function showAlert(msg, type) {
    const box = document.getElementById("alert-box");
    if (!box) return;
    box.innerHTML = '<div class="alert alert-' + type + '">' + esc(msg) + '</div>';
    setTimeout(() => { box.innerHTML = ""; }, 4000);
}

// Show the result of an action ({ok, msg}) as green or red alert
function showResult(result) {
    if (result) showAlert(result.msg, result.ok ? "success" : "danger");
}

// Make text safe before putting it inside HTML
function esc(text) {
    return String(text).replace(/[&<>"']/g, c =>
        ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
}

// Open / Filling / Full badge
function statusBadge(registered, capacity) {
    if (registered >= capacity) return '<span class="badge badge-red">Full</span>';
    if (registered >= capacity * 0.8) return '<span class="badge badge-orange">Filling</span>';
    return '<span class="badge badge-green">Open</span>';
}
