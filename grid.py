# grid.py

import geopandas as gpd
import shapely.geometry
import numpy as np

class GridGenerator:
    def __init__(self, grid_size=1):
        self.grid_size = grid_size
        self.grid = {}
        self.land = None

    def load_land_data(self, geojson_path):
        self.land = gpd.read_file(geojson_path)

    def generate_world_grid(self):
        lat_range = np.arange(-90, 90, self.grid_size)
        lon_range = np.arange(-180, 180, self.grid_size)

        for lat in lat_range:
            for lon in lon_range:
                center = shapely.geometry.Point(lon + self.grid_size/2, lat + self.grid_size/2)
                is_land = False
                if self.land is not None:
                    for polygon in self.land.geometry:
                        if polygon.contains(center):
                            is_land = True
                            break
                self.grid[(lat, lon)] = {
                    'lat': lat,
                    'lon': lon,
                    'is_land': is_land
                }
        print(f"Generated grid with {len(self.grid)} cells.")

    def get_grid(self):
        return self.grid
