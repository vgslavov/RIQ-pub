package edu.umkc.sce.dbis.anask;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.OptionBuilder;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.PosixParser;
import org.apache.commons.io.FilenameUtils;

import com.hp.hpl.jena.query.Dataset;
import com.hp.hpl.jena.query.Query;
import com.hp.hpl.jena.query.QueryExecution;
import com.hp.hpl.jena.query.QueryExecutionFactory;
import com.hp.hpl.jena.query.QueryFactory;
import com.hp.hpl.jena.query.ReadWrite;
import com.hp.hpl.jena.query.ResultSet;
import com.hp.hpl.jena.query.ResultSetFactory;
import com.hp.hpl.jena.query.ResultSetFormatter;
import com.hp.hpl.jena.query.ResultSetRewindable;
import com.hp.hpl.jena.sparql.util.ResultSetUtils;
import com.hp.hpl.jena.tdb.TDBFactory;


public class Querier {

    public static void main(String[] args) {
        Options options = getProgramOptions();

        // another way to pass args (for testing):
        //args = new String[]{"-results","xml", "-dir","/Users/anask/Downloads/RIQ/Data10/dir","-file","/Users/anask/Downloads/RIQ/Data10/dir/ndF.txt","-q","/Users/anask/Downloads/RIQ/Data10/nq1.q"};

        String candidates_file;
        String query_file;
        String optqueries_dir = null;
        String index_dir;
        String format;
        boolean print_count = false;

        HelpFormatter formatter = new HelpFormatter();
        if (args.length <= 1) {
            formatter.printHelp("querier", options);
            return;
        } else {
            CommandLineParser parser = new PosixParser();
            try {
                CommandLine line = parser.parse(options, args);

                query_file = line.getOptionValue("query");

                if (line.hasOption("optqueries")) {
                    optqueries_dir = line.getOptionValue("optqueries");
                }

                candidates_file = line.getOptionValue("candidates");
                index_dir = line.getOptionValue("index");

                format = line.getOptionValue("results").toUpperCase();

                if (!format.equals("TEXT") && !format.equals("CSV") &&
                    !format.equals("XML") && !format.equals("TSV")) {

                    System.err.println("warn: unrecognized format: " +
                                       format);
                }

                if (line.hasOption("c"))
                    print_count = true;

                if (line.hasOption("h")) {
                    formatter.printHelp("querier", options);
                    return;
                }
            } catch (ParseException exp) {
                System.err.println("error: parsing failed. " +
                                   exp.getMessage());
                formatter.printHelp("querier", options);
                return;

            }
        }

        File candidates = new File(candidates_file);

        File[] tdb_list = null;
        try {
            tdb_list = readFile(candidates, index_dir);
        } catch (IOException exp) {
            System.err.println("error: IOException caught");
            return;
        }

        System.err.println("FILES FOUND: " + tdb_list.length);

        HashMap<String, String> cand2query = new HashMap<String, String>();
        Query query = QueryFactory.read(query_file);
        if (optqueries_dir != null) {
            System.err.println("READING optqueries in dir: " +
                               FilenameUtils.getName(optqueries_dir));
            File optqueries = new File(optqueries_dir);
            File[] optqueries_list = optqueries.listFiles();
            if (tdb_list.length != optqueries_list.length) {
                System.err.println("warn: number of candidates and " +
                                   "number of rewritten queries differ");
                //return;
            }

            for (File rqmod: optqueries_list) {
                if (rqmod.isFile()) {
                    String path = rqmod.getAbsolutePath();
                    String file_name = FilenameUtils.getName(path);
                    //System.err.println(file_name);
                    String[] rqmod_parts = file_name.split("\\.");
                    // XXX: format is datahub.g.q4.1001000001000000.rqmod
                    int cand_idx = rqmod_parts.length - 2;
                    //System.err.println(rqmod_parts[cand_idx]);
                    // TODO: store only file_name?
                    cand2query.put(rqmod_parts[cand_idx], path);
                } else {
                    System.err.println("warn: not a file");
                }
            }

            //tdb_list = optqueries_dir;
        } else {
            System.err.println("READING single query file: " +
                               FilenameUtils.getName(query_file));
        }

        int total_results = 0;
        ResultSet results = null;

        System.err.println("QUERYING INDICES ..");
        for (File loc: tdb_list) {
            // System.err.println(loc);
            if (loc.isDirectory()) {
                // get path of the index to be queried
                String path = loc.getAbsolutePath();
                String file_name = FilenameUtils.getName(path);

                // connect to the index
                Dataset dataset = TDBFactory.createDataset(path);
                dataset.begin(ReadWrite.READ);

                try {
                    // issue query
                    QueryExecution qexec = null;
                    if (optqueries_dir != null &&
                        cand2query.containsKey(file_name)) {

                        String rqmod_file = cand2query.get(file_name);
                        System.err.println("Executing rqmod query: " +
                                           FilenameUtils.getName(rqmod_file));
                        Query rqmod_query = QueryFactory.read(rqmod_file);
                        qexec = QueryExecutionFactory.create(rqmod_query,
                                                             dataset);
                    } else {
                        System.err.println("Executing original query: " +
                                           FilenameUtils.getName(query_file));
                        qexec = QueryExecutionFactory.create(query, dataset);
                    }

                    try {
                        ResultSet result = qexec.execSelect();

                        // reset rewindable stream when counting
                        if (print_count) {
                            ResultSetRewindable rsrw = ResultSetFactory.
                                                       copyResults(result);
                            int candidate_results = rsrw.size();

                            System.err.println("MATCHES FOUND IN " +
                                               file_name + ": " +
                                               candidate_results);
                            total_results += candidate_results;

                            rsrw.reset();
                            if (results != null)
                                results = ResultSetUtils.union(rsrw, results);
                            else
                                results = ResultSetFactory.copyResults(rsrw);

                        } else {
                            if (results != null)
                                results = ResultSetUtils.union(result, results);
                            else
                                results = ResultSetFactory.copyResults(result);
                        }
                    } finally {
                        qexec.close();
                    }
                } finally {
                    dataset.end();
                    // not necessary
                    //dataset.close();
                }
            } else {
                System.err.println("warn: not a dir");
            }
        }

        if (print_count)
            System.err.println("TOTAL MATCHES FOUND: " + total_results);

        if (format.equals("XML"))
            ResultSetFormatter.outputAsXML(results);
        else if (format.equals("CSV"))
            ResultSetFormatter.outputAsCSV(results);
        else if (format.equals("TSV"))
            ResultSetFormatter.outputAsTSV(results);
        else
            System.out.print(ResultSetFormatter.asText(results));
    }

    private static Options getProgramOptions() {
        Options options = new Options();

        Option help = new Option("h", "print this message");
        //Option m = new Option("m", false,
        //                      "output query matches for each index");
        //Option f = new Option("f", false,"format output as a text table");
        Option count = new Option("c", false,
                                  "count the number of query matches");
        Option index = OptionBuilder.withArgName("dir").hasArg(true).withDescription("candidate indices are stored in this dir").isRequired(true).create("index");
        Option candidates = OptionBuilder.withArgName("file").hasArg(true).withDescription("list of candidates").isRequired(true).create("candidates");
        Option query = OptionBuilder.withArgName("file").hasArg(true).withDescription("SPARQL query").isRequired(true).create("query");
        Option opt_queries = OptionBuilder.withArgName("dir").hasArg(true).withDescription("dir with optimized SPARQL queries (one per candidate)").isRequired(false).create("optqueries");
        Option results = OptionBuilder.withArgName("format").hasArg(true).withDescription("Results format: text (default), XML, CSV, TSV").isRequired(false).create("results");

        options.addOption(help);
        //options.addOption(m);
        //options.addOption(f);
        options.addOption(count);
        options.addOption(index);
        options.addOption(candidates);
        options.addOption(query);
        options.addOption(opt_queries);
        options.addOption(results);

        return options;
    }

    private static File[] readFile(File fin, String indDir) throws IOException {
        FileInputStream fis = new FileInputStream(fin);
        ArrayList<File> files = new ArrayList<File>();

        BufferedReader br = new BufferedReader(new InputStreamReader(fis));

        String line = null;
        while ((line = br.readLine()) != null) {
            String path = indDir + "/" + line;
            //System.err.println(path);
            files.add(new File(path));
        }

        br.close();

        return files.toArray(new File[files.size()]);
    }
}
