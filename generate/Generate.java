/**
 * create random map (with solution).
 * 
 * @param map
 */
public class Generate {
	
	static int randomSlides = 70;
	static int COLUMN = 4;
	static int ROW = 4;
	
	public static void main(String[] args)  {
		for (int i = 0; i < 100; ++i) {
			int[][] initMap = new int[COLUMN][ROW];
			Generate.initializeMap(initMap);
			for(int r = 0; r < ROW; r++) {
				for (int c = 0; c < COLUMN; c++) {
					System.out.print(initMap[r][c]);
					System.out.print(" ");
				}
			}
			System.out.println();
		}
	}

	public static void initializeMap(int[][] map) {
		for (int column = 0; column < COLUMN; column++) {
			for (int row = 0; row < ROW; row++) {
				map[column][row] = column * 4 + row;
			}
		}
		int zeroC = 0; // local only for this method. similar to that in Fringe
						// class.
		int zeroR = 0;

		for (int i = 0; i < randomSlides; i++) {
			double rand = Math.random();
			if (rand < 0.25) {
				if (zeroR != 0) {
					map[zeroC][zeroR] = map[zeroC][zeroR - 1];
					map[zeroC][zeroR - 1] = 0;
					zeroR--;
				}
			} else if (rand < 0.5) {
				if (zeroR != ROW-1) {
					map[zeroC][zeroR] = map[zeroC][zeroR + 1];
					map[zeroC][zeroR + 1] = 0;
					zeroR++;
				}
			} else if (rand < 0.75) {
				if (zeroC != 0) {
					map[zeroC][zeroR] = map[zeroC - 1][zeroR];
					map[zeroC - 1][zeroR] = 0;
					zeroC--;
				}
			} else {
				if (zeroC != COLUMN -1) {
					map[zeroC][zeroR] = map[zeroC + 1][zeroR];
					map[zeroC + 1][zeroR] = 0;
					zeroC++;
				}
			}
		}
	}

}
