<!DOCTYPE html>
<html>
<head>
  <title>Ship Routing with A* Pathfinding</title>
  <meta charset="utf-8" />
  <link rel="stylesheet" href="https://unpkg.com/leaflet/dist/leaflet.css" />
  <style>
    #map { height: 100vh; width: 100vw; }
    .ship-icon { font-size: 24px; }
  </style>
</head>
<body>

<div id="map"></div>

<script src="https://unpkg.com/leaflet/dist/leaflet.js"></script>
<script src="https://unpkg.com/@turf/turf@6.5.0/turf.min.js"></script>

<script>
let map = L.map('map').setView([20, 0], 2);
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { attribution: '© OpenStreetMap contributors' }).addTo(map);

let startMarker = null, endMarker = null, shipMarker = null, pathLine = null;
let countries = [];
const gridSize = 1; // 1 degree grid (can be made smaller for more accuracy)
let walkableGrid = [];

fetch('https://raw.githubusercontent.com/johan/world.geo.json/master/countries.geo.json')
.then(res => res.json())
.then(data => { countries = data.features; });

map.on('click', function(e) {
  if (!startMarker) {
    startMarker = L.marker(e.latlng, { title: "Start Point" }).addTo(map);
  } else if (!endMarker) {
    endMarker = L.marker(e.latlng, { title: "End Point" }).addTo(map);
    generateGridAndPath();
  }
});

function generateGridAndPath() {
  walkableGrid = [];

  for (let lat = -90; lat <= 90; lat += gridSize) {
    for (let lng = -180; lng <= 180; lng += gridSize) {
      const cellCenter = turf.point([lng + gridSize/2, lat + gridSize/2]);
      let isObstacle = false;

      for (let country of countries) {
        if (turf.booleanPointInPolygon(cellCenter, country)) {
          isObstacle = true;
          break;
        }
      }
      walkableGrid.push({
        lat: lat + gridSize/2,
        lng: lng + gridSize/2,
        walkable: !isObstacle
      });
    }
  }

  findPath();
}

function findPath() {
  let openSet = [];
  let closedSet = [];
  let start = nearestNode(startMarker.getLatLng());
  let end = nearestNode(endMarker.getLatLng());

  start.g = 0;
  start.f = heuristic(start, end);
  openSet.push(start);

  while (openSet.length > 0) {
    openSet.sort((a, b) => a.f - b.f);
    let current = openSet.shift();

    if (current.lat === end.lat && current.lng === end.lng) {
      reconstructPath(current);
      return;
    }

    closedSet.push(current);

    for (let neighbor of neighbors(current)) {
      if (!neighbor.walkable || closedSet.find(n => n.lat === neighbor.lat && n.lng === neighbor.lng)) {
        continue;
      }
      let tentative_g = current.g + distance(current, neighbor);

      let openNeighbor = openSet.find(n => n.lat === neighbor.lat && n.lng === neighbor.lng);
      if (!openNeighbor || tentative_g < neighbor.g) {
        neighbor.parent = current;
        neighbor.g = tentative_g;
        neighbor.f = tentative_g + heuristic(neighbor, end);

        if (!openNeighbor) openSet.push(neighbor);
      }
    }
  }

  alert('No path found!');
}

function neighbors(node) {
  let deltas = [
    [0, gridSize], [gridSize, 0], [0, -gridSize], [-gridSize, 0],
    [gridSize, gridSize], [-gridSize, -gridSize], [gridSize, -gridSize], [-gridSize, gridSize]
  ];
  let results = [];

  for (let [dLat, dLng] of deltas) {
    let lat = node.lat + dLat;
    let lng = node.lng + dLng;
    let neighbor = walkableGrid.find(n => Math.abs(n.lat - lat) < 0.001 && Math.abs(n.lng - lng) < 0.001);
    if (neighbor) results.push(neighbor);
  }
  return results;
}

function heuristic(a, b) {
  return Math.abs(a.lat - b.lat) + Math.abs(a.lng - b.lng);
}

function distance(a, b) {
  return Math.sqrt(Math.pow(a.lat - b.lat, 2) + Math.pow(a.lng - b.lng, 2));
}

function nearestNode(latlng) {
  let minDist = Infinity;
  let nearest = null;
  walkableGrid.forEach(n => {
    let d = Math.sqrt(Math.pow(latlng.lat - n.lat, 2) + Math.pow(latlng.lng - n.lng, 2));
    if (d < minDist) {
      minDist = d;
      nearest = {...n};
    }
  });
  return nearest;
}

function reconstructPath(node) {
  let path = [];
  while (node) {
    path.push([node.lat, node.lng]);
    node = node.parent;
  }
  path.reverse();
  
  pathLine = L.polyline(path, { color: 'blue' }).addTo(map);
  animateShip(path);
}

function animateShip(path) {
  let shipIcon = L.divIcon({ className: 'ship-icon', html: '🚢' });
  shipMarker = L.marker(path[0], { icon: shipIcon }).addTo(map);

  let i = 0;
  let interval = setInterval(() => {
    if (i >= path.length) { clearInterval(interval); return; }
    shipMarker.setLatLng(path[i]);
    i++;
  }, 300);
}
</script>

</body>
</html>
