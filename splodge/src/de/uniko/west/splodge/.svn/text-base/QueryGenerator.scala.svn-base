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

package de.uniko.west.splodge

import java.io.File
import java.io.FileInputStream
import java.util.zip.GZIPInputStream

import scala.Array.canBuildFrom
import scala.collection.mutable.HashMap
import scala.collection.mutable.Map
import scala.io.Source
import scala.util.Random

object QueryGenerator {

  //  val FILE = "btc2011-condensed-ctx-list.txt"
  val PATH_STATS = "path-stats.gz"
  val PRED_STATS = "predicate-stats.gz";
  val PREDICATES = "predicate-list.gz";
  val CONTEXTS   = "context-list.gz";

  val rand = new Random(42)

  val cIndex = new HashMap[Int, String]
  val pIndex = new HashMap[Int, String]
  val pStats = new HashMap[Int, HashMap[Int, HashMap[Int, HashMap[Int, Array[Int]]]]];

  def open(file: File): Source = {
    if (file.getName.endsWith(".gz")) {
      Source.fromInputStream(new GZIPInputStream(new FileInputStream(file)));
    } else {
      Source.fromFile(file)
    }
  }

  def loadDict(lines: Iterator[String], index: Map[Int, String]): Unit = {
    var n = 1; for (line <- lines) { index += (n -> line); n += 1 };
  }

  /**
   * Loads predicate path statistics, i.e. co-occurring predicates in
   * connected RDF triples (s, p1, x), (x, p2, o) in source s1 and s2.
   * Input lines from plain text file contain (p1, p2, s1, s2) followed by 
   * statistics (#res, #t1: |(s,p1,x)@s1)|, #t2: |(x,p2,o)@s2)|) which
   * are put into a nested hash map: map->p1->s1->p2->s2=(#res, #t1, #t2).
   */
  def loadPathStats(lines: Iterator[String]): Unit = {
    for (line <- lines) {
      val Array(p1, p2, s1, s2, res, t1, t2) = line.split(' ') map (_.toInt)
      // TODO: handle same source and cross sources path stats
      if (s1 != s2)
    	  pStats.getOrElseUpdate(p1, HashMap()).getOrElseUpdate(s1, HashMap()).getOrElseUpdate(p2, HashMap()).getOrElseUpdate(s2.toInt, Array(res, t1, t2));
    }
  }

  def pickRandomPath(map: Map[Int, _]): List[Int] = {
    val key = map.keys.toList(rand.nextInt(map.size))
    map.get(key).get match {
      case m: Map[Int, _] => key :: pickRandomPath(m)
      case _ => List(key)
    }
  }

  def createPathJoin(length: Int): List[(Int, Int)] = {
    var joins = List[(Int, Int)]()

    while (joins.length < length) {
      if (joins.length == 0) {
        val List(p1, c1, p2, c2) = pickRandomPath(pStats);
        joins ++= List((p1, c1), (p2, c2))
      }
      val (p, c) = joins.last
      if (pStats.contains(p) && pStats.get(p).get.contains(c)) {
        val List(p2, c2) = pickRandomPath(pStats.get(p).get.get(c).get)
        joins ++= List((p2, c2))
      } else {
        joins = List[(Int, Int)]()
      }
    }
    joins
  }
  
  def printSPARQL(path: List[(Int, Int)]) = {
    var n = 0;
    val pattern = path map { x => n += 1; "?var%s %s ?var%s".format(n, pIndex.get(x._1).get, n+1)}
    printf("SELECT * WHERE { %s }\n", pattern.mkString(" . ")) 
  }

  def main(args: Array[String]): Unit = {
    val now = System.currentTimeMillis

    println("loading statistics")
    loadPathStats(open(new File(PATH_STATS)).getLines)
    loadDict(open(new File(PREDICATES)).getLines, pIndex)
    loadDict(open(new File(CONTEXTS)).getLines, cIndex)

    for (i <- 1 to 10) printSPARQL(createPathJoin(4))

    println("total time taken: %f seconds".format((System.currentTimeMillis - now) / 1e3))
  }

}