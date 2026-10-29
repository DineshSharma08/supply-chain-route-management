
const MAIN_WAREHOUSE = "Delhi";
const STORAGE_KEY = "supplyFlowData";

let data = loadData();
let toastTimer;

function $(id) {
    return document.getElementById(id);
}

function loadData() {
    try {
        const saved = localStorage.getItem(STORAGE_KEY);

        if (saved) {
            const parsed = JSON.parse(saved);

            if (Array.isArray(parsed.locations) &&
                Array.isArray(parsed.routes) &&
                Array.isArray(parsed.deliveries) &&
                Array.isArray(parsed.completedDeliveries)) {

                if (!parsed.locations.some(
                    name => name.toLowerCase() === MAIN_WAREHOUSE.toLowerCase()
                )) {
                    parsed.locations.unshift(MAIN_WAREHOUSE);
                }

                return parsed;
            }
        }
    } catch (error) {
        console.log("Saved data could not be loaded.");
    }

    return {
        locations: ["Delhi", "Jaipur", "Mumbai"],
        routes: [
            { from: "Delhi", to: "Jaipur", distance: 280 },
            { from: "Delhi", to: "Mumbai", distance: 1400 }
        ],
        deliveries: [],
        completedDeliveries: []
    };
}

function saveData() {
    try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(data));
    } catch (error) {
        showMessage("Could not save data in this browser.", true);
    }
}

function safeText(value) {
    return String(value).replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function showMessage(message, error = false) {
    const box = $("statusMessage");
    box.textContent = message;
    box.classList.remove("hidden", "error");

    if (error) {
        box.classList.add("error");
    }

    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => box.classList.add("hidden"), 3500);
}

function renderAll() {
    if (!data.locations.some(
        name => name.toLowerCase() === MAIN_WAREHOUSE.toLowerCase()
    )) {
        data.locations.unshift(MAIN_WAREHOUSE);
    }

    renderStats();
    renderLocations();
    renderRoutes();
    renderSelects();
    renderDeliveries();
    saveData();
}

function renderStats() {
    $("locationCount").textContent = data.locations.length;
    $("routeCount").textContent = data.routes.length;
    $("deliveryCount").textContent = data.deliveries.length;
    $("completedCount").textContent = data.completedDeliveries.length;
}

function countRoutes(location) {
    return data.routes.filter(route =>
        route.from === location || route.to === location
    ).length;
}

function renderLocations() {
    const list = $("locationList");
    list.innerHTML = "";

    $("locationListLabel").textContent =
        data.locations.length + " location(s)";

    data.locations.forEach(location => {
        const isMain = location.toLowerCase() ===
            MAIN_WAREHOUSE.toLowerCase();

        const item = document.createElement("div");
        item.className = "managed-item";

        item.innerHTML = `
            <span class="location-icon ${isMain ? "warehouse" : ""}">
                ${isMain ? "🏭" : "📍"}
            </span>
            <span class="managed-main">
                <strong>${safeText(location)}</strong>
                <small>${isMain ? "Central dispatch point" :
                    countRoutes(location) + " connected route(s)"}</small>
            </span>
            ${isMain
                ? '<span class="protected-chip">MAIN WAREHOUSE</span>'
                : `<button class="delete-button" type="button"
                    data-location="${safeText(location)}">Delete</button>`}
        `;

        list.appendChild(item);
    });
}

function renderRoutes() {
    const list = $("routeList");
    list.innerHTML = "";

    $("emptyRoutes").classList.toggle("hidden", data.routes.length > 0);

    data.routes.forEach((route, index) => {
        const row = document.createElement("tr");

        row.innerHTML = `
            <td><span class="route-name">
                ${safeText(route.from)} ↔ ${safeText(route.to)}
            </span></td>
            <td><span class="distance-chip">${route.distance} km</span></td>
            <td><button class="delete-button" type="button"
                data-route="${index}">Delete</button></td>
        `;

        list.appendChild(row);
    });
}

function renderSelects() {
    const ids = [
        "sourceLocation",
        "destinationLocation",
        "routeFrom",
        "routeTo",
        "deliveryDestination"
    ];

    ids.forEach(id => {
        const select = $(id);
        const previous = select.value;
        select.innerHTML = "";

        data.locations.forEach(location => {
            const option = document.createElement("option");
            option.value = location;
            option.textContent = location +
                (location === MAIN_WAREHOUSE ? " (Main Warehouse)" : "");
            select.appendChild(option);
        });

        if (data.locations.includes(previous)) {
            select.value = previous;
        }
    });

    if (data.locations.includes(MAIN_WAREHOUSE)) {
        $("sourceLocation").value = MAIN_WAREHOUSE;
    }

    if (data.locations.length > 1 &&
        $("destinationLocation").value === MAIN_WAREHOUSE) {
        $("destinationLocation").value =
            data.locations.find(name => name !== MAIN_WAREHOUSE);
    }

    if ($("routeFrom").value === $("routeTo").value &&
        data.locations.length > 1) {
        $("routeTo").value =
            data.locations.find(name => name !== $("routeFrom").value);
    }
}

function renderDeliveries() {
    const pending = $("deliveryList");
    const completed = $("completedDeliveryList");

    pending.innerHTML = "";
    completed.innerHTML = "";

    $("pendingLabel").textContent = data.deliveries.length + " pending";
    $("completedLabel").textContent =
        data.completedDeliveries.length + " completed";

    $("processDeliveryBtn").disabled = data.deliveries.length === 0;
    $("processDeliveryBtn").style.opacity =
        data.deliveries.length === 0 ? "0.55" : "1";

    if (data.deliveries.length === 0) {
        pending.innerHTML =
            '<p class="empty">No pending deliveries. Add one above.</p>';
    }

    data.deliveries.forEach((delivery, index) => {
        const item = document.createElement("div");
        item.className = "delivery-item";

        item.innerHTML = `
            <span class="package-icon">📦</span>
            <span class="delivery-main">
                <strong>${safeText(delivery.name)}</strong>
                <small>Dispatch: Delhi → ${safeText(delivery.destination)}</small>
            </span>
            <span class="delivery-badge">
                ${index === 0 ? "Next up" : "Queued"}
            </span>
        `;

        pending.appendChild(item);
    });

    if (data.completedDeliveries.length === 0) {
        completed.innerHTML =
            '<p class="empty">Completed deliveries will appear here.</p>';
    }

    [...data.completedDeliveries].reverse().forEach(delivery => {
        const item = document.createElement("div");
        item.className = "delivery-item";

        item.innerHTML = `
            <span class="package-icon" style="background:#e4f9ef">✓</span>
            <span class="delivery-main">
                <strong>${safeText(delivery.name)}</strong>
                <small>Delhi → ${safeText(delivery.destination)}</small>
            </span>
            <span class="delivery-badge completed">Completed</span>
        `;

        completed.appendChild(item);
    });
}

$("locationForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const name = $("locationName").value.trim();

    if (!name) return;

    if (data.locations.some(
        location => location.toLowerCase() === name.toLowerCase()
    )) {
        showMessage("This location already exists.", true);
        return;
    }

    data.locations.push(name);
    $("locationName").value = "";

    renderAll();
    showMessage(name + " added successfully.");
});

$("routeForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const from = $("routeFrom").value;
    const to = $("routeTo").value;
    const distance = Number($("routeDistance").value);

    if (!from || !to) {
        showMessage("Add at least two locations first.", true);
        return;
    }

    if (from === to) {
        showMessage("Choose two different locations.", true);
        return;
    }

    if (!Number.isFinite(distance) || distance <= 0) {
        showMessage("Enter a valid distance greater than zero.", true);
        return;
    }

    const exists = data.routes.some(route =>
        (route.from === from && route.to === to) ||
        (route.from === to && route.to === from)
    );

    if (exists) {
        showMessage("This route already exists. Delete it before adding it again.", true);
        return;
    }

    data.routes.push({ from, to, distance });
    $("routeDistance").value = "";

    renderAll();
    showMessage("Route added successfully.");
});

$("routeList").addEventListener("click", function(event) {
    const button = event.target.closest("[data-route]");
    if (!button) return;

    const index = Number(button.dataset.route);
    const route = data.routes[index];

    if (!route) return;

    if (confirm(`Delete route ${route.from} ↔ ${route.to}?`)) {
        data.routes.splice(index, 1);
        renderAll();
        showMessage("Route deleted successfully.");
    }
});

$("locationList").addEventListener("click", function(event) {
    const button = event.target.closest("[data-location]");
    if (!button) return;

    const name = button.dataset.location;

    if (name.toLowerCase() === MAIN_WAREHOUSE.toLowerCase()) {
        showMessage("Delhi is the main warehouse and cannot be deleted.", true);
        return;
    }

    const linkedRoutes = countRoutes(name);

    if (linkedRoutes > 0) {
        showMessage(
            `Delete the ${linkedRoutes} connected route(s) before deleting ${name}.`,
            true
        );
        return;
    }

    const usedInPending = data.deliveries.some(
        delivery => delivery.destination === name
    );

    const usedInCompleted = data.completedDeliveries.some(
        delivery => delivery.destination === name
    );

    if (usedInPending || usedInCompleted) {
        showMessage("This location is used in delivery history and cannot be deleted.", true);
        return;
    }

    if (confirm(`Delete location "${name}"?`)) {
        data.locations = data.locations.filter(location => location !== name);
        renderAll();
        showMessage(name + " deleted successfully.");
    }
});

function shortestPath(start, end) {
    if (start === end) return { path: [start], distance: 0 };

    const distances = {};
    const previous = {};
    const unvisited = new Set(data.locations);

    data.locations.forEach(location => {
        distances[location] = Infinity;
        previous[location] = null;
    });

    distances[start] = 0;

    while (unvisited.size > 0) {
        let current = null;
        let minimum = Infinity;

        unvisited.forEach(location => {
            if (distances[location] < minimum) {
                minimum = distances[location];
                current = location;
            }
        });

        if (current === null) break;
        if (current === end) break;

        unvisited.delete(current);

        data.routes.forEach(route => {
            let neighbour = null;

            if (route.from === current) neighbour = route.to;
            if (route.to === current) neighbour = route.from;

            if (neighbour && unvisited.has(neighbour)) {
                const newDistance = distances[current] + Number(route.distance);

                if (newDistance < distances[neighbour]) {
                    distances[neighbour] = newDistance;
                    previous[neighbour] = current;
                }
            }
        });
    }

    if (!Number.isFinite(distances[end])) return null;

    const path = [];
    let current = end;

    while (current !== null) {
        path.unshift(current);
        current = previous[current];
    }

    return { path, distance: distances[end] };
}

$("shortestPathForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const start = $("sourceLocation").value;
    const end = $("destinationLocation").value;
    const resultBox = $("shortestResult");

    resultBox.classList.remove("hidden", "error");

    const result = shortestPath(start, end);

    if (!result) {
        resultBox.classList.add("error");
        resultBox.innerHTML =
            `No connected route exists from ${safeText(start)} to ${safeText(end)}. Add connecting routes and try again.`;
        return;
    }

    resultBox.innerHTML = `
        <b>Shortest route found!</b><br>
        <span class="result-path">${result.path.map(safeText).join(" → ")}</span><br>
        Total distance: <b>${result.distance} km</b>
    `;
});

$("deliveryForm").addEventListener("submit", function(event) {
    event.preventDefault();

    const name = $("deliveryName").value.trim();
    const destination = $("deliveryDestination").value;

    if (!name || !destination) return;

    data.deliveries.push({ name, destination });
    $("deliveryName").value = "";

    renderAll();
    showMessage("Delivery added to the pending queue.");
});

$("processDeliveryBtn").addEventListener("click", function() {
    if (data.deliveries.length === 0) {
        showMessage("No pending deliveries to process.", true);
        return;
    }

    const delivery = data.deliveries.shift();
    data.completedDeliveries.push({
        name: delivery.name,
        destination: delivery.destination
    });

    renderAll();
    showMessage(delivery.name + " processed successfully.");
});

$("toggleRoutesBtn").addEventListener("click", function() {
    const area = $("routesArea");
    const isHidden = area.classList.toggle("hidden");

    this.textContent = isHidden ? "View All Routes ↓" : "Hide Routes ↑";
});

renderAll();
