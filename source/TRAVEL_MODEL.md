# Version 3.0 travel model

Version 3.0 uses the 18 full-time Big Ten member universities listed by the
conference in 2025–2026: Illinois, Indiana, Iowa, Maryland, Michigan, Michigan
State, Minnesota, Nebraska, Northwestern, Ohio State, Oregon, Penn State,
Purdue, Rutgers, UCLA, USC, Washington, and Wisconsin.

Each destination is assigned an approximate latitude and longitude for its
main campus. The game calculates great-circle distance with the haversine
formula and converts it to a game fare:

    fare = clamp(5 + round(miles / 125), 5, 25)

This makes nearby campuses such as UCLA and USC cost $5 while cross-country
travel approaches, but never exceeds, $25. Big Ten Headquarters is represented
near Rosemont, Illinois and uses the same distance calculation.

Sources consulted:

- Big Ten Conference, “What Are the Big Ten Schools Known For?”
  https://bigten.org/article/blt8282033f4f246c5d/
- Big Ten Conference, “University of Oregon, UCLA, USC and University of
  Washington Officially Join Big Ten Conference”
  https://bigten.org/article/blt257197176c07308f/

Coordinates are intentionally approximate gameplay data, not navigation data.
