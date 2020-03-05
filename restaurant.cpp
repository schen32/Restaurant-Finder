#include "restaurant.h"

/*
	Sets *ptr to the i'th restaurant. If this restaurant is already in the cache,
	it just copies it directly from the cache to *ptr. Otherwise, it fetches
	the block containing the i'th restaurant and stores it in the cache before
	setting *ptr to it.
*/
void getRestaurant(restaurant* ptr, int i, Sd2Card* card, RestCache* cache) {
	// calculate the block with the i'th restaurant
	uint32_t block = REST_START_BLOCK + i/8;

	// if this is not the cached block, read the block from the card
	if (block != cache->cachedBlock) {
		while (!card->readBlock(block, (uint8_t*) cache->block)) {
			Serial.print("readblock failed, try again");
		}
		cache->cachedBlock = block;
	}

	// either way, we have the correct block so just get the restaurant
	*ptr = cache->block[i%8];
}

// Swaps the two restaurants (which is why they are pass by reference).
void swap(RestDist& r1, RestDist& r2) {
	RestDist tmp = r1;
	r1 = r2;
	r2 = tmp;
}

// Insertion sort to sort the restaurants.
void insertionSort(RestDist restaurants[], int counter) {
	// Invariant: at the start of iteration i, the
	// array restaurants[0 .. i-1] is sorted.
	for (int i = 1; i < counter; ++i) {
		// Swap restaurant[i] back through the sorted list restaurants[0 .. i-1]
		// until it finds its place.
		for (int j = i; j > 0 && restaurants[j].dist < restaurants[j-1].dist; --j) {
			swap(restaurants[j-1], restaurants[j]);
		}
	}
}

int pivot(RestDist restaurants[], int n, int pi) {
	swap(restaurants[pi], restaurants[n-1]);
	int lo = 0;
	int hi = n - 2;

	while (lo <= hi) {
		if (restaurants[hi].dist > restaurants[n-1].dist) {
			hi--;
		}
		else if (restaurants[lo].dist <= restaurants[n-1].dist) {
			lo++;
		}
		else {
			swap(restaurants[lo], restaurants[hi]);
		}
	}

	swap(restaurants[lo], restaurants[n-1]);
	return lo;
}

void quickSort(RestDist restaurants[], int n) {
	if (n < 1) {
		return;
	}

	int pi = n/2;
	int newPi = pivot(restaurants, n, pi);

	quickSort(restaurants, newPi);
	quickSort(restaurants + (newPi + 1), n - (newPi + 1));
}

// Computes the manhattan distance between two points (x1, y1) and (x2, y2).
int16_t manhattan(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	return abs(x1-x2) + abs(y1-y2);
}

/*
	Fetches all restaurants from the card, saves their RestDist information
	in restaurants[], and then sorts them based on their distance to the
	point on the map represented by the MapView.
*/
void getAndSortRestaurants(const MapView& mv, RestDist restaurants[], Sd2Card* card, RestCache* cache, int sort, int rating) {
	restaurant r;
	int start;
	int stop;
	int time;
	int counter = 0;

	// First get all the restaurants and store their corresponding RestDist information.
	for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int convertedRating = max(floor((r.rating + 1)/2), 1);
		if (convertedRating >= rating) {
			restaurants[counter].index = i;
			restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
																		mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
			counter++;
		}

	}

	//Selecting which sorting alogrithm to use
	if (sort == 0) {
		start = millis();
		quickSort(restaurants, counter);
		stop = millis();
		time = stop - start;
		Serial.print("qsort running time: ");
		Serial.print(time);
		Serial.print("ms");
	}
	else if (sort == 1) {
		start = millis();
		insertionSort(restaurants, counter);
		stop = millis();
		time = stop - start;
		Serial.print("isort running time: ");
		Serial.print(time);
		Serial.print("ms");
	} 
	else if (sort == 2) {
		start = millis();
		quickSort(restaurants, counter);
		stop = millis();
		time = stop - start;
		Serial.print("qsort running time: ");
		Serial.print(time);
		Serial.print("ms");
		Serial.println("");

		counter = 0;
		for (int i = 0; i < NUM_RESTAURANTS; ++i) {
			getRestaurant(&r, i, card, cache);
			int convertedRating = max(floor((r.rating + 1)/2), 1);
			if (convertedRating >= rating) {
				restaurants[counter].index = i;
				restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
																			mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
				counter++;
			}

		}

		start = millis();
		insertionSort(restaurants, counter);
		stop = millis();
		time = stop - start;
		Serial.print("isort running time: ");
		Serial.print(time);
		Serial.print("ms");
	}
}
