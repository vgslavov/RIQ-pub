/*
 * This file is part of SPLODGE.
 * Copyright 2012 Olaf Goerlitz
 *
 * SPLODGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SPLODGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SPLODGE.  If not, see <http://www.gnu.org/licenses/>.
 */
package de.uniko.west.splodge;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.zip.GZIPInputStream;

/**
 * SPLODGE Query Generator.
 * 
 * @author Olaf Goerlitz (goerlitz@uni-koblenz.de)
 */
public class QueryGen {
	
	private static final String DATA_DIR   = "testdata/";
	private static final String PATH_STATS = "path-stats.gz";
	private static final String PREDICATES = "predicate-list.gz";
	private static final String PRED_STATS = "predicate-stats.gz";
	private static final String CONTEXTS   = "context-list.gz";
	private static final Random RAND = new Random(42);
	private static final int QUERIES = 10;
	
	private Map<Integer, Map<Integer, Integer>> predStats = new HashMap<Integer, Map<Integer,Integer>>();
	private PathStatistics pStats = new PathStatistics(RAND);
	private Map<Integer, String> pIndex = new HashMap<Integer, String>();
	private Map<Integer, String> cIndex = new HashMap<Integer, String>();
	
	
	public QueryGen() {}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		QueryGen gen = new QueryGen();

		// loading statistics
		gen.loadStatistics();
		gen.loadDisctionaries();
		gen.loadPredicateStats();

		// loading configuration
		
		// generating queries
		gen.generateQueries(QUERIES);
	}
	
	public void loadPredicateStats() {
		try {
			BufferedReader reader = openReader(new File(DATA_DIR + PRED_STATS));
			String line = "";
			while ((line = reader.readLine()) != null) {
				String[] parts = line.split(" "); // predicateID, #triple, #source, #triplesPerSource[]
				int predID = Integer.parseInt(parts[0]);
				Map<Integer, Integer> pMap = predStats.get(predID);
				if (pMap == null) {
					pMap = new HashMap<Integer, Integer>();
					predStats.put(predID, pMap);
				}
				
				// process individual triple counts per source
				for (String count : parts[3].split(",")) {
					String[] keyValue = count.split(":");
					int key = Integer.parseInt(keyValue[0]);
					int val = Integer.parseInt(keyValue[1]);
					pMap.put(key, val);
				}
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void loadStatistics() {
		try {
			pStats.loadPathStatistics(openReader(new File(DATA_DIR + PATH_STATS)));
		} catch (FileNotFoundException e) {
			System.err.println("Error: file not found. " + e.getMessage());
			System.exit(1);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void loadDisctionaries() {
		try {
			loadDictionary(openReader(new File(DATA_DIR + PREDICATES)), pIndex);
			loadDictionary(openReader(new File(DATA_DIR + CONTEXTS)), cIndex);
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void loadDictionary(BufferedReader reader, Map<Integer, String> map) throws IOException {
		int pos = 0;
		String line = "";
		
		while ((line = reader.readLine()) != null) {
			map.put(pos++, line);
		}
	}
	
	public void computePathSelectivity() {
		
	}
	
	public void generatePathJoin(int numPatterns, int numSources) {
		
		int retries = 0;
		List<Pair> pathJoin = new ArrayList<Pair>();
		
		while (pathJoin.size() < numPatterns) {

			if (pathJoin.size() == 0) {
				int[] choice = pStats.pickPathJoin();
				pathJoin.add(new Pair(choice[0], choice[1]));
				pathJoin.add(new Pair(choice[2], choice[3]));
				//				System.out.println("new path: " + Arrays.toString(choice));
				continue;
			}

			Pair last = pathJoin.get(pathJoin.size()-1);
			if (pStats.exists(last.predicate, last.source)) {
				int[] choice = pStats.pickPathJoin(last.predicate, last.source);
				pathJoin.add(new Pair(choice[2], choice[3]));
			} else {
				// no path available
				pathJoin = new ArrayList<Pair>();
				retries++;
			}

		}

		System.out.println("new path: " + pathJoin + ", retries: " + retries);
		System.out.println("sparql: " + toSPARQL(pathJoin, pIndex));
//		for (Pair p : pathJoin) {
//			System.out.println("tp: " + pIndex.get(p.predicate) + " @ " + cIndex.get(p.source));
//		}

		pathJoin = new ArrayList<Pair>();
	}
	
	public void generateQueries(int runs) {
		
		for (int i =0; i< runs; i++) {
			generatePathJoin(4, 4);
		}
	}
	
	private static final String toSPARQL(List<Pair> pathJoin, Map<Integer, String> pIndex) {
		int var = 0;
		StringBuilder builder = new StringBuilder("SELECT * WHERE { ");
		for (Pair p : pathJoin) {
			builder.append("?var").append(var++);
			builder.append(" ").append(pIndex.get(p.predicate)).append(" ?var");
			builder.append(var).append(" . ");
		}
		builder.append(" } ");
		return builder.toString();
	}
	
	private static final BufferedReader openReader(File file) throws FileNotFoundException, IOException {
		InputStream in = new FileInputStream(file);
		if (file.getName().endsWith(".gz"))
			in = new GZIPInputStream(in);
		return new BufferedReader(new InputStreamReader(in));
	}
	
	public class Pair {
		int predicate;
		int source;
		
		public Pair(int predicate, int source) {
			this.predicate = predicate;
			this.source = source;
		}
		
		public String toString() {
			return "[" + predicate + ", " + source + "]";
		}
	}
	
}
