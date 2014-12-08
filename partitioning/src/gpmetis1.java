import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import jdbm.PrimaryTreeMap;
import jdbm.RecordManager;
import jdbm.RecordManagerFactory;
/**
 *
 * @author db985
 */
public class gpmetis1 {

    BufferedWriter bwGraph;
    //Name of the file that has IDs in subjects and Objects
    String PreGraph="PreGraph";
    //Name of the file that has Metis understandable format
    String MetisFile="MetisFile";
    String PreRdfType = "PreRdfType";
    int VERTICES;
/**
     * @param args the command line arguments
     * //args[0] -> Path to the original connected Graph (SplFile)
       //args[1] -> List of Unique Subjects and Objects (vertices)
       //args[2] -> Destination path
     * //args[3] -> Actual File that is to be converted to a Graph (ActualFile)
     * //args[4] -> File that was separated out from the Actual file (RdfType)
     * //args[5] -> Type of Stragery used
     * //args[6] -> Use Edge Weights (Y/N)
     * //args[7] -> Path to the file that has edge weights
     * //args[8] -> Complete Dataset in GIS Format (GIS_nhop)
     * //args[9] -> List of Predicates
     * //args[10] -> Number of vertices
     */
    public static void main(String[] args) {
        // TODO code application logic here
        if(args.length!=11){
            System.out.println("Invalid Command line arguments!\n");
            System.out.println("Arguments: <ConnectedGraphFile> <Vertices> <DestinationPath> <ActualGraph> <RdfType> <StrategyType> <Y/N> <EdgeWeightsFile> <WholeDataSet_GISFormat> <PredicatesList> <NoOfVertices>");
        }
        else{
            gpmetis1 drObject=new gpmetis1();
            try{
                System.out.println("gpmetis1.java ---> Execution started!");
                //get the time when actual program execution starts
                drObject.CurrentTime();

                //core method
                System.out.println();
                System.out.println("In ConvertToIDs..");
                //code that converts SplFile to PreGraph
                drObject.ConvertToIDs(args[0],args[1],args[2], args[3], args[4], args[5], args[8], args[9], args[10]); //commented just for now!!!

                System.out.println();
                System.out.println("In ConvertToGraph..");
                if(args[5].equals("T1")) {
                    //code that converts PreGraph to MetisFile
                    drObject.ConvertToG(args[0], args[1], args[2], args[5], args[6], args[7]);
                }
                else {
                    //code that converts PreGraph to MetisFile
                    drObject.ConvertToG(args[3], args[1], args[2], args[5], args[6], args[7]);
                }
                System.out.println("gpmetis1.java ---> Execution completed!");
                //get the time when execution ends
                drObject.CurrentTime();

            }
            catch(Exception e){
                System.out.println("Invalid Partitions/Arguments entered!");
                return;
            }
        }
    }

    public void ConvertToIDs(String File1,String File2,String Loc, String ActualFile, String SepGraph, String Type, String GIS_nhop, String Predicates, String NoVertices){
        try{
            Process p3=Runtime.getRuntime().exec("mkdir "+Loc+"Databases/2 ");
            p3.waitFor();

            // Get Manager singleton
            RecordManager recman = RecordManagerFactory.createRecordManager(Loc+"Databases/2/hugedata1");
            //create hashmap for unique subjects and objects
            PrimaryTreeMap<String, Integer> MMap=insertMap(File2, recman, Loc+"Databases/2/hugemap1");
            System.out.println("Entries in Map2:"+MMap.size());
            VERTICES = Integer.parseInt(NoVertices);
            HashMap<String, Integer> hmTemp = new HashMap<String, Integer>();
            BufferedReader brPred = new BufferedReader(new FileReader(Predicates));
            String Pred = null;
            int next = VERTICES + 1;
            while((Pred = brPred.readLine()) != null) {
                hmTemp.put(Pred, next);
                next++;
            }

            if(Type.equals("T1")) {
                //replace subjects and objects with corresponding IDs
                WriteIds(File1, Loc, MMap, GIS_nhop, hmTemp);
            }
            else {

                Sep_SubGraph(Loc, File1, ActualFile, SepGraph);
                //replace subjects and objects with corresponding IDs
                WriteIds(ActualFile, Loc, MMap, GIS_nhop, hmTemp);
                WriteIdTypes(SepGraph, Loc, MMap, GIS_nhop, hmTemp);
            }

            // Close the manager
            recman.close();
            hmTemp.clear();
            System.out.println();
            System.out.println("Creating Sorted PreGraph Files..");
            p3=Runtime.getRuntime().exec(Loc+"sort.sh" + " " + Loc+PreGraph);
            p3.waitFor();
            if(!Type.equals("T1")) {
                System.out.println();
                System.out.println("Creating Sorted PreRdfType Files..");
                p3=Runtime.getRuntime().exec(Loc+"sort.sh" + " " + Loc+PreRdfType);
                p3.waitFor();
            }
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }

    public void Sep_SubGraph(String DestLoc, String file1, String ActualFile, String SepGraph) {
        try {
            BufferedWriter bwRdfType = new BufferedWriter(new FileWriter(SepGraph));
            BufferedWriter bwMainGraph = new BufferedWriter(new FileWriter(ActualFile));

            BufferedReader brSplFile=new BufferedReader(new FileReader(file1));
            String triple;
            while((triple = brSplFile.readLine())!=null) {
                int i=triple.indexOf(' ');
                String Subject=triple.substring(0,i);
                String y=triple.substring(i+1);
                int P=y.indexOf(' ');
                String Predicate=y.substring(0,P);
                String z=y.substring(P+1);
                int Ob=z.lastIndexOf('.');
                String Object=z.substring(0,Ob-1);

                if(Object.contains("\"")) {
                    bwRdfType.write(triple);
                    bwRdfType.newLine();
                }
                else {
                    if(Predicate.equals("rdf:type") || Predicate.equals("rdfs:subClassOf")){
                        bwRdfType.write(triple);
                        bwRdfType.newLine();
                    }
                    else{
                        bwMainGraph.write(triple);
                        bwMainGraph.newLine();
                    }
                }
            }
            bwRdfType.close();
            bwMainGraph.close();
            brSplFile.close();
        }
        catch(Exception ex){
            System.out.println("Error while removing a subgraph!");
            ex.printStackTrace();
        }
    }

    //replace subjects and objects with corresponding IDs
    public void WriteIds(String file1, String DestLoc, PrimaryTreeMap<String, Integer> map11, String GIS_nhop, HashMap<String, Integer> hmTemp){
        String triple;
        BufferedWriter bwOutput = null;
        BufferedWriter bwGIS_nhop = null;
        try{
            bwGIS_nhop = new BufferedWriter(new FileWriter(GIS_nhop, true));
            bwGIS_nhop.write("t # 0");
            bwGIS_nhop.newLine();

            bwOutput= new BufferedWriter(new FileWriter(DestLoc + PreGraph,false));
            //filereader to process complete Sorted file
            BufferedReader brMain=new BufferedReader(new FileReader(file1));
            StringBuilder NewTriple, GISTriple;
            while((triple=brMain.readLine())!=null){
                int i=triple.indexOf(' ');
                String Subject=triple.substring(0,i);
                String y=triple.substring(i+1);
                int P=y.indexOf(' ');
                String Predicate=y.substring(0,P);
                String z=y.substring(P+1);
                int Ob=z.lastIndexOf('.');
                String Object=z.substring(0,Ob-1);

                NewTriple=new StringBuilder();
                int id1 = map11.get(Subject);
                int id2 = hmTemp.get(Predicate);
                int id3 = map11.get(Object);
                NewTriple.append(id1).append(" ");
                NewTriple.append(id2).append(" ");
                NewTriple.append(id3).append(" .");

                bwOutput.write(NewTriple.toString());
                bwOutput.newLine();

                //GIS Triple
                GISTriple=new StringBuilder();
                GISTriple.append("u ").append(id1).append(" ").append(id3).append(" ").append(id2);
                bwGIS_nhop.write(GISTriple.toString());
                bwGIS_nhop.newLine();
            }
            brMain.close();
            bwGIS_nhop.close();
            bwOutput.close();
        }
        catch(IOException ex){
            ex.printStackTrace();
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }

    //replace subjects and objects with corresponding IDs for RdfType Graph
    public void WriteIdTypes(String file1, String DestLoc, PrimaryTreeMap<String, Integer> map11, String GIS_nhop, HashMap<String, Integer> hmTemp){
        String triple;
        BufferedWriter bwOutput;
        BufferedWriter bwGIS_nhop = null;
        try{
            bwGIS_nhop = new BufferedWriter(new FileWriter(GIS_nhop, true));
            bwOutput= new BufferedWriter(new FileWriter(DestLoc + PreRdfType,false));
            //filereader to process complete Sorted file
            BufferedReader brMain=new BufferedReader(new FileReader(file1));
            StringBuilder NewTriple, GISTriple;
            while((triple=brMain.readLine())!=null){
                int i=triple.indexOf(' ');
                String Subject=triple.substring(0,i);
                String y=triple.substring(i+1);
                int P=y.indexOf(' ');
                String Predicate=y.substring(0,P);
                String z=y.substring(P+1);
                int Ob=z.lastIndexOf('.');
                String Object=z.substring(0,Ob-1);

                NewTriple=new StringBuilder();
                int id1 = map11.get(Subject);
                int id2 = hmTemp.get(Predicate);
                NewTriple.append(id1).append(" ");
                NewTriple.append(id2).append(" ");
                int id3 = map11.get(Object);
                if(Object.contains("\"")) {
                    NewTriple.append(Object).append(" .");
                }
                else {
                    NewTriple.append(id3).append(" .");
                }

                bwOutput.write(NewTriple.toString());
                bwOutput.newLine();

                GISTriple=new StringBuilder();
                GISTriple.append("u ").append(id1).append(" ").append(id3).append(" ").append(id2);
                bwGIS_nhop.write(GISTriple.toString());
                bwGIS_nhop.newLine();
            }
            bwOutput.close();
            bwGIS_nhop.close();
            brMain.close();
        }
        catch(IOException ex){
            ex.printStackTrace();
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }

    public void ConvertToG(String file1, String file2, String file3, String Type, String GiveEdgeWeights, String EdgeWeightFile){
        try{
            String s;
            int edges=0;

            Process p3=Runtime.getRuntime().exec("wc -l "+file1);
            p3.waitFor();
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p3.getInputStream()));
            while ((s = stdInput.readLine()) != null) {
                StringTokenizer st=new StringTokenizer(s);
                s=st.nextToken();
                edges = Integer.parseInt(s);
            }            
            stdInput.close();

            //for edge weights
            HashMap<String,Integer> hm=null;
            if(GiveEdgeWeights.equals("Y")){
                BufferedReader brEdgeWeights = new BufferedReader(new FileReader(EdgeWeightFile));
                hm=new HashMap();
                String edge = "";
                while((edge=brEdgeWeights.readLine())!=null) {
                    StringTokenizer tzr=new StringTokenizer(edge);
                    hm.put(tzr.nextToken(), Integer.parseInt(tzr.nextToken()));
                }
                brEdgeWeights.close();
            }

            System.out.println();
            System.out.println("*** ... Step1: Creating Metis Input ...");
            Process p33=Runtime.getRuntime().exec("mkdir "+file3+"Databases/3");
            p33.waitFor();

            // Get Manager singleton
            RecordManager recman = RecordManagerFactory.createRecordManager(file3+"Databases/3/hugedata1");
            //create hashmap for unique subjects and objects
            PrimaryTreeMap<Integer, String> Map = recman.treeMap(file3+"Databases/3/hugemap1");


            System.out.println("***** ... Working on Subjects ...");
            BufferedReader brPreGraph=new BufferedReader(new FileReader(file3+PreGraph+"1"));
            int count=1;
            String triple1=brPreGraph.readLine();
            String triple2;
            StringTokenizer strtkn1 = new StringTokenizer(triple1);
            int subjID1=Integer.parseInt(strtkn1.nextToken());
            String Predicate1 = strtkn1.nextToken();
            String Object1 = strtkn1.nextToken();
            StringBuilder value = new StringBuilder();
            value.append(Object1);
            if(GiveEdgeWeights.equals("Y")){
                if(hm.containsKey(Predicate1))
                    value.append(" ").append(hm.get(Predicate1).toString());
                else
                    value.append(" ").append("1");
            }
            while((triple2=brPreGraph.readLine())!=null){
                StringTokenizer strtkn2 = new StringTokenizer(triple2);
                int subjID2 = Integer.parseInt(strtkn2.nextToken());
                String Predicate2 = strtkn2.nextToken();
                String Object2 = strtkn2.nextToken();
                if(subjID1==subjID2){
                    value.append(" ").append(Object2);
                    if(GiveEdgeWeights.equals("Y")){
                        if(hm.containsKey(Predicate2))
                            value.append(" ").append(hm.get(Predicate2).toString());
                        else
                            value.append(" ").append("1");
                    }
                }
                else{
                    Map.put(subjID1,value.toString());
                    value.setLength(0);
                    value.append(Object2);
                    if(GiveEdgeWeights.equals("Y")){
                        if(hm.containsKey(Predicate2))
                            value.append(" ").append(hm.get(Predicate2).toString());
                        else
                            value.append(" ").append("1");
                    }
                    subjID1=subjID2;
                }
                count++;
                if(count%1e5==0)
                        recman.commit();
                if(count%1e7==0)
                    System.out.println("Committed at : "+count);
            }
            Map.put(subjID1,value.toString());
            brPreGraph.close();
            recman.commit();

            recman.clearCache();

            bwGraph= new BufferedWriter(new FileWriter(file3 + MetisFile, false));
            TreeMap<Integer,Integer> Vertex_Weight_Map = null;
            boolean S_T3 = false;

            if(GiveEdgeWeights.equals("Y")){
                //Strategy 3
                if(Type.equals("T3") || Type.equals("T4")) {
                    S_T3 = true;
                    bwGraph.write(VERTICES+" "+edges+" "+"011");
                    Vertex_Weight_Map = GetVertexWeights(file3, Type, VERTICES);
                }
                else
                    bwGraph.write(VERTICES+" "+edges+" "+"001");
            }
            else {
                //Strategy 3
                if(Type.equals("T3") || Type.equals("T4")) {
                    S_T3 = true;
                    bwGraph.write(VERTICES+" "+edges+" "+"010");
                    Vertex_Weight_Map = GetVertexWeights(file3, Type, VERTICES);
                }
                else
                    bwGraph.write(VERTICES+" "+edges);
            }
            bwGraph.newLine();


            System.out.println("***** ... Working on Objects ...");
            brPreGraph=new BufferedReader(new FileReader(file3+PreGraph+"2"));
            count=1;

            triple1=brPreGraph.readLine();
            strtkn1 = new StringTokenizer(triple1);
            String Subject1 = strtkn1.nextToken();
            Predicate1 = strtkn1.nextToken();
            int objID1 = Integer.parseInt(strtkn1.nextToken());
            String OldVal="";
            StringBuilder sb=new StringBuilder();
            sb.append(Subject1);
            if(GiveEdgeWeights.equals("Y")){
                if(hm.containsKey(Predicate1))
                    sb.append(" ").append(hm.get(Predicate1).toString());
                else
                    sb.append(" ").append("1");
            }
            //if Strategy 3
            if(S_T3) {
                while((triple2=brPreGraph.readLine())!=null){
                    StringTokenizer strtkn2 = new StringTokenizer(triple2);
                    String Subject2 = strtkn2.nextToken();
                    String Predicate2 = strtkn2.nextToken();
                    int objID2 = Integer.parseInt(strtkn2.nextToken());
                    //runs only for the first time.
                    if(count==1){
                        if(!Map.isEmpty()){
                            int i=Map.firstKey();

                            if(i<objID1) {
                                //doesn't occur in subject and object
                                for(int kl=1;kl<i;kl++) {
                                    int weight = Vertex_Weight_Map.get(kl);
                                    bwGraph.write(Integer.toString(weight));
                                    Vertex_Weight_Map.remove(kl);
                                    bwGraph.newLine();
                                }
                            }
			    else {
				for(int kl=1;kl<objID1;kl++) {
				    int weight = Vertex_Weight_Map.get(kl);
                                    bwGraph.write(Integer.toString(weight));
                                    Vertex_Weight_Map.remove(kl);
                                    bwGraph.newLine();
				}
			    }
                            //if the vertex doesnot occur as object
                            while(i<objID1){
                                int weight = Vertex_Weight_Map.get(i);
                                Vertex_Weight_Map.remove(i);
                                if(Map.containsKey(i)){
                                    OldVal = Map.get(i);
                                    bwGraph.write(Integer.toString(weight) + " " + OldVal);
                                    Map.remove(i);
                                }
                                else {
                                    bwGraph.write(Integer.toString(weight));
                                }
                                bwGraph.newLine();
                                i++;
                            }
                        }
                    }
                    if(objID1==objID2){
                        sb.append(" ").append(Subject2);
                        if(GiveEdgeWeights.equals("Y")){
                            if(hm.containsKey(Predicate2))
                                sb.append(" ").append(hm.get(Predicate2).toString());
                            else
                                sb.append(" ").append("1");
                        }
                    }
                    else{
                        if(Map.containsKey(objID1)){
                            OldVal = Map.get(objID1);
                            sb.append(" ").append(OldVal);
                            Map.remove(objID1);
                        }
                        int weight = Vertex_Weight_Map.get(objID1);
                        Vertex_Weight_Map.remove(objID1);
                        bwGraph.write(Integer.toString(weight) + " " + sb.toString());
                        bwGraph.newLine();
                        sb.setLength(0);
                        sb.append(Subject2);
                        if(GiveEdgeWeights.equals("Y")){
                            if(hm.containsKey(Predicate2))
                                sb.append(" ").append(hm.get(Predicate2).toString());
                            else
                                sb.append(" ").append("1");
                        }
                        while(objID2!=objID1+1){
                            objID1++;
                            weight = Vertex_Weight_Map.get(objID1);
                            Vertex_Weight_Map.remove(objID1);
                            if(Map.containsKey(objID1))
                            {
                                OldVal = Map.get(objID1);
                                bwGraph.write(Integer.toString(weight) + " " + OldVal);
                                Map.remove(objID1);
                            }
                            else{
                                bwGraph.write(Integer.toString(weight));
                            }
                            bwGraph.newLine();
                        }
                        objID1 = objID2;
                    }
                    count++;
                    if(count%1e7==0)
                        System.out.println("Committed at : "+count);
                }

                if(!Map.isEmpty()) {
                    if(Map.containsKey(objID1)) {
                        OldVal = Map.get(objID1);
                        sb.append(" ").append(OldVal);
                        //value = OldVal + " " + value;
                        Map.remove(objID1);
                    }
                }
                int weight = Vertex_Weight_Map.get(objID1);
                Vertex_Weight_Map.remove(objID1);
                bwGraph.write(Integer.toString(weight) + " " + sb.toString());
                bwGraph.newLine();

                objID1++;
                while(objID1<=VERTICES) {
                    if(!Map.isEmpty()) {
                        int i = Map.firstKey();
                        while(objID1<i) {
                            weight = Vertex_Weight_Map.get(objID1);
                            Vertex_Weight_Map.remove(objID1);
                            bwGraph.write(Integer.toString(weight));
                            bwGraph.newLine();
                            objID1++;
                        }
                        weight = Vertex_Weight_Map.get(objID1);
                        Vertex_Weight_Map.remove(objID1);
                        OldVal = Map.get(objID1);
                        Map.remove(objID1);
                        bwGraph.write(Integer.toString(weight) + " " + OldVal);
                    }
                    else {
                        weight = Vertex_Weight_Map.get(objID1);
                        Vertex_Weight_Map.remove(objID1);
                        bwGraph.write(Integer.toString(weight));
                    }
                    bwGraph.newLine();
                    objID1++;
                }
            }
            else {
                while((triple2=brPreGraph.readLine())!=null){
                    StringTokenizer strtkn2 = new StringTokenizer(triple2);
                    String Subject2 = strtkn2.nextToken();
                    String Predicate2 = strtkn2.nextToken();
                    int objID2 = Integer.parseInt(strtkn2.nextToken());
                    //runs only for the first time.
                    if(count==1){
                        if(!Map.isEmpty()){
                            int i=Map.firstKey();
                            if(i<=objID1) {
                                //doesn't occur in subject and object
                                for(int kl=1;kl<i;kl++) {
                                    bwGraph.newLine();
                                }
                            }
			    else if (i>objID1) {
				for(int kl=1;kl<objID1;kl++)
				    bwGraph.newLine();
			    }
                            //if the vertex doesnot occur as object
                            while(i<objID1){
                                if(Map.containsKey(i)){
                                    OldVal = Map.get(i);
                                    bwGraph.write(OldVal);
                                    Map.remove(i);
                                }
                                bwGraph.newLine();
                                i++;
                            }
                        }
                    }
                    if(objID1==objID2){
                        sb.append(" ").append(Subject2);
                        if(GiveEdgeWeights.equals("Y")){
                            if(hm.containsKey(Predicate2))
                                sb.append(" ").append(hm.get(Predicate2).toString());
                            else
                                sb.append(" ").append("1");
                        }
                    }
                    else{
                        if(Map.containsKey(objID1)){
                            OldVal = Map.get(objID1);
                            sb.append(" ").append(OldVal);
                            Map.remove(objID1);
                        }
                        bwGraph.write(sb.toString());
                        bwGraph.newLine();
                        sb.setLength(0);
                        sb.append(Subject2);
                        if(GiveEdgeWeights.equals("Y")){
                            if(hm.containsKey(Predicate2))
                                sb.append(" ").append(hm.get(Predicate2).toString());
                            else
                                sb.append(" ").append("1");
                        }
                        while(objID2!=objID1+1){
                            objID1++;
                            if(Map.containsKey(objID1))
                            {
                                OldVal = Map.get(objID1);
                                bwGraph.write(OldVal);
                                Map.remove(objID1);
                            }
                            bwGraph.newLine();
                        }
                        objID1 = objID2;
                    }
                    count++;
                    if(count%1e7==0)
                        System.out.println("Committed at : "+count);
                }

                if(!Map.isEmpty()) {
                    if(Map.containsKey(objID1)) {
                        OldVal = Map.get(objID1);
                        sb.append(" ").append(OldVal);
                        //value = OldVal + " " + value;
                        Map.remove(objID1);
                    }
                }
                bwGraph.write(sb.toString());
                bwGraph.newLine();

                objID1++;
                while(objID1<=VERTICES) {
                    if(!Map.isEmpty()) {
                        int i = Map.firstKey();
                        while(objID1<i) {
                            bwGraph.newLine();
                            objID1++;
                        }
                        OldVal = Map.get(objID1);
                        Map.remove(objID1);
                        bwGraph.write(OldVal);
                    }
                    bwGraph.newLine();
                    objID1++;
                }
            }
	    
            bwGraph.close();
            brPreGraph.close();
            recman.close();
        }
        catch(Exception e){
            e.printStackTrace();
            System.out.println("ERROR In ConvertToGraph Method!!!!!!!!!!!!!!!!!");
        }
    }

    //get vertex weights
    public TreeMap<Integer,Integer> GetVertexWeights(String DestLoc, String Type, int vertices){
        System.out.println("Calculating the weights of vertices...");
        TreeMap<Integer,Integer> tm=new TreeMap<Integer,Integer>();
        try{
            if(Type.equals("T3")) {
                BufferedReader brPreG11 = new BufferedReader(new FileReader(DestLoc+PreGraph+"1"));
                String Triple=null;
                int Count = 1;
                int Num=0;
                int Pst=0;
                boolean val=false;
                StringTokenizer strtkr;

                while(true) {
                    if(Triple!=null){
                        if(Pst==Count){
                            val = true;
                            Num++;
                        }
                        else {
                            while(Pst>=Count+1) {
                                tm.put(Count, 0);
                                Count++;
                            }
                            val = true;
                            Num++;
                        }
                    }
                    //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
                    while((Triple = brPreG11.readLine()) != null){
                        strtkr=new StringTokenizer(Triple);
                        Pst = Integer.parseInt(strtkr.nextToken());
                        if(Pst == Count){
                            val = true;
                            Num++;
                        }
                        else if(!val){
                            while(Pst>=Count+1) {
                                tm.put(Count, 0);
                                Count++;
                            }
                            val = true;
                            Num++;
                        }
                        else{
                            tm.put(Count, Num);
                            Count++;
                            Num = 0;
                            val = false;
                            break;
                        }
                    }
                    if(Triple==null) {
                        tm.put(Count, Num);
                        Count++;
                        break;
                    }
                }
                while(Count<=vertices){
                    tm.put(Count, 0);
                    Count++;
                }
                brPreG11.close();

                BufferedReader brPreG22 = new BufferedReader(new FileReader(DestLoc+PreGraph+"2"));
		int Prv = -1;
		if((Triple = brPreG22.readLine()) != null)
		{
                    Triple=brPreG22.readLine();
                    strtkr = new StringTokenizer(Triple);
                    strtkr.nextToken();
                    strtkr.nextToken();
                    Prv = Integer.parseInt(strtkr.nextToken());
  		}
                Num = 1;
                //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
                while((Triple = brPreG22.readLine()) != null){
                    strtkr=new StringTokenizer(Triple);
                    strtkr.nextToken();
                    strtkr.nextToken();
                    Pst = Integer.parseInt(strtkr.nextToken());
                    if(Pst == Prv){
                        Num++;
                    }
                    else{
                        if(tm.containsKey(Prv)) {
                            int vl=tm.get(Prv);
                            tm.put(Prv, vl+Num);
                        }
                        else
                            tm.put(Prv, Num);
                        Prv = Pst;
                        Num = 1;
                    }
                }
                brPreG22.close();

                BufferedReader brPreR11 = new BufferedReader(new FileReader(DestLoc+PreRdfType+"1"));
		if((Triple = brPreR11.readLine()) != null) {
                    Triple=brPreR11.readLine();
                    strtkr = new StringTokenizer(Triple);
                    Prv = Integer.parseInt(strtkr.nextToken());
		}
                Num = 1;
                //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
                while((Triple = brPreR11.readLine()) != null){
                    strtkr=new StringTokenizer(Triple);
                    Pst = Integer.parseInt(strtkr.nextToken());
                    if(Pst == Prv){
                        Num++;
                    }
                    else{
                        if(tm.containsKey(Prv)) {
                            int vl=tm.get(Prv);
                            tm.put(Prv, vl+Num);
                        }
                        else
                            tm.put(Prv, Num);
                        Prv = Pst;
                        Num = 1;
                    }
                }
                brPreR11.close();
            }
            else if(Type.equals("T4")) {
                BufferedReader brPreR11 = new BufferedReader(new FileReader(DestLoc+PreRdfType+"1"));
                String Triple=null;
                int Count = 1;
                int Num=0;
                int Pst=0;
                boolean val=false;
                StringTokenizer strtkr;
                while(true) {
                    if(Triple!=null){
                        if(Pst==Count){
                            val = true;
                            Num++;
                        }
                        else {
                            while(Pst>=Count+1) {
                                tm.put(Count, 0);
                                Count++;
                            }
                            val = true;
                            Num++;
                        }
                    }
                    //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
                    while((Triple = brPreR11.readLine()) != null){
                        strtkr=new StringTokenizer(Triple);
                        Pst = Integer.parseInt(strtkr.nextToken());
                        if(Pst == Count){
                            val = true;
                            Num++;
                        }
                        else if(!val){
                            while(Pst>=Count+1) {
                                tm.put(Count, 0);
                                Count++;
                            }
                            val = true;
                            Num++;
                        }
                        else{
                            tm.put(Count, Num);
                            Count++;
                            Num = 0;
                            val = false;
                            break;
                        }
                    }
                    if(Triple==null) {
                        tm.put(Count, Num);
                        Count++;
                        break;
                    }
                }
                while(Count<=vertices) {
                    tm.put(Count, 0);
                    Count++;
                }
                brPreR11.close();
            }

            String Triple = null;
            StringTokenizer strtkr;
            int Prv=0;
            int Pst=0;
            BufferedReader brPreR22 = new BufferedReader(new FileReader(DestLoc+PreRdfType+"2"));
            while((Triple=brPreR22.readLine())!=null) {
                strtkr = new StringTokenizer(Triple);
                strtkr.nextToken();
                strtkr.nextToken();
                String x=strtkr.nextToken();
                if(x.startsWith("\""))
                    continue;
                else{
                    Prv = Integer.parseInt(x);
                    break;
                }

            }
            int Num = 1;
            //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
            while((Triple = brPreR22.readLine()) != null){
                strtkr=new StringTokenizer(Triple);
                strtkr.nextToken();
                strtkr.nextToken();
                Pst = Integer.parseInt(strtkr.nextToken());
                if(Pst == Prv){
                    Num++;
                }
                else{
                    if(tm.containsKey(Prv)) {
                        int vl=tm.get(Prv);
                        tm.put(Prv, vl+Num);
                    }
                    else
                        tm.put(Prv, Num);
                    Prv = Pst;
                    Num = 1;
                }
            }
            brPreR22.close();
        }
        catch(Exception ex){
            System.out.println("Error while assigning weights to vertices!!");
            ex.printStackTrace();
        }
        return tm;
    }

    //method that inserts unique subjects and objects into TreeMap
    public PrimaryTreeMap<String, Integer> insertMap(String VerticesFile, RecordManager recman, String Loc){
        PrimaryTreeMap<String, Integer> map = recman.treeMap(Loc);
        String rec;
        int i=1;
        try{
            BufferedReader bufRdr=new BufferedReader(new FileReader(VerticesFile));
            while((rec=bufRdr.readLine())!=null){
                map.put(rec, i);
                i++;
                if(i%1e5==0)
                    recman.commit();
                if(i%1e7==0)
                    System.out.println("Committed at : "+i);
            }
            bufRdr.close();
            recman.commit();
            return map;
        }
        catch(IOException ex){
            System.out.print("Error in insertMap");
            ex.printStackTrace();
        }
        catch(Exception e){
            e.printStackTrace();
        }
        return null;
    }

    //method that inserts unique subjects and objects into TreeMap
    public PrimaryTreeMap<Integer,String> insertMap44(String VerticesFile, RecordManager recman, String Loc){
        PrimaryTreeMap<Integer,String> map = recman.treeMap(Loc);
        String rec;
        int i=1;
        try{
            BufferedReader bufRdr=new BufferedReader(new FileReader(VerticesFile));
            while((rec=bufRdr.readLine())!=null){
                map.put(i, rec);
                i++;
                if(i%1e5==0)
                    recman.commit();
                if(i%1e7==0)
                    System.out.println("Committed at : "+i);
            }
            bufRdr.close();
            recman.commit();
            return map;
        }
        catch(IOException ex){
            System.out.print("Error in insertMap");
            ex.printStackTrace();
        }
        catch(Exception e){
            e.printStackTrace();
        }
        return null;
    }

    //method to get current time in yyyy-MM-dd HH:mm:ss.SSS format
    public void CurrentTime(){
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        String dateString = sdf.format(new Date());
        System.out.println("CurrentTime = " + dateString);
    }
}

