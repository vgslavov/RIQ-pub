import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import jdbm.PrimaryHashMap;
import jdbm.PrimaryTreeMap;
import jdbm.RecordManager;
import jdbm.RecordManagerFactory;
/**
 *
 * @author db985
 */
public class nHop_1 {
    BufferedWriter bwGraphPartition;
    boolean check=false;
    String edgeslist = "edgesID";
    /**
     * @param args the command line arguments
     * args[0] -> File that has triples from Actual File in ID's format (PreGraph)
     * args[1] -> Output from Metis (hmetisFile.part.[?]/MetisFile.part.[?])
     * args[2] -> Destination path
     * args[3] -> Number to Partitions
     * args[4] -> Strategy Type (T1/T2)
     * args[5] -> File that has triples from RdfType File in ID's format (PreRdfType)
     * args[6] -> Input to gpmetis (MetisFile)
     * args[7] -> QueueFile (queuefile)
     * args[8] -> Use Edge Weights (Y/N)
     * args[9] -> vertices file
     */
    public static void main(String[] args) {
        // TODO code application logic here
        if(args.length!=10){
            System.out.println("Invalid Command line arguments!\n");
            System.out.println("Arguments: <PreGraph> <hmetis_output/gpmetis_output> <DestinationPath> <NoOfPartitions> <Strategy_type> <PreRdfType> <Metis_input> <queuefile> <Include_EdgeWeights(Y/N)> <Vertices File>");
        }
        else{
            nHop_1 drObject=new nHop_1();
            try{
                System.out.println("nHop_1.java ---> Execution started!");
                //get the time when actual program execution starts
                drObject.CurrentTime();
                //core method
                System.out.println();
                System.out.println("In GraphPartitioning..");
                //Using Metis output file, partitions all triples to their corresponding buckets
                //drObject.FinalPartition(args[0], args[1], args[2], args[3], args[4], args[5], args[9]); //<-- commented for now
                //System.out.println();
		//drObject.GetQueue(args[2], args[7], Integer.parseInt(args[3]));
                //if(drObject.check){
                    System.out.println("Getting list of egdecuts..");
                    drObject.GetEdgeCuts(args[2], args[4], args[0], args[1], args[6], args[7], args[8]);
		  //}
               System.out.println();
               System.out.println("nHop_1.java ---> Execution completed!");
                
		
                //get the time when execution ends
                drObject.CurrentTime();
            }
            catch(Exception e){
                System.out.println("Invalid Partitions/Arguments entered!");
                return;
            }
        }
    }

    //Partition ID Triples into it's corresponding partition number according to Metis output file
    public void FinalPartition(String Pregraphfile, String MetisOutput, String file3 , String NoOfPartitions, String Type, String PreRdffile, String File2){
        try{
           Process p2 = Runtime.getRuntime().exec("mkdir "+file3+"Databases/15 ");
	   p2.waitFor();
	   
	   //Get Manager singleton
	   RecordManager recman = RecordManagerFactory.createRecordManager(file3+"Databases/15/hugedata1");
	   //create hashmap for metis output
	   PrimaryHashMap<Integer, Integer> hmPartition=insertMap(MetisOutput, recman, file3+"Databases/15/hugemap1");
	   System.out.println("Entries in Map:"+hmPartition.size());	    

    	   RecordManager recman1 = RecordManagerFactory.createRecordManager(file3+"Databases/2/hugedata1");
	   PrimaryTreeMap<String, Integer> MMap=insertMap44(File2, recman1, file3+"Databases/2/hugemap1");
	   System.out.println("Entries in Vertices map:"+MMap.size());

            //map to store metisoutput file
/*            HashMap<Integer, Integer> hmPartition = new HashMap();
            BufferedReader brMap=new BufferedReader(new FileReader(MetisOutput));
            String st = null;
            int id = 1;
            while((st = brMap.readLine()) != null) {
                hmPartition.put(id, Integer.parseInt(st));
                id++;
            }
            brMap.close();
*/
            String fileId;
            int NumberOfPartitions = Integer.parseInt(NoOfPartitions);
            for(int i=0;i<NumberOfPartitions;i++){
                check = false;
            	//reads two files; one sorted on Subjects and the other sorted on Objects
            	BufferedReader brPreG11 = new BufferedReader(new FileReader(Pregraphfile+"1"));
            	BufferedReader brPreG22 = new BufferedReader(new FileReader(Pregraphfile+"2"));
                BufferedReader brPreR11 =null;
                BufferedReader brPreR22 = null;
                if(!Type.equals("T1")) {
                    brPreR11 = new BufferedReader(new FileReader(PreRdffile+"1"));
                    brPreR22 = new BufferedReader(new FileReader(PreRdffile+"2"));
                }
                BufferedReader brMain=new BufferedReader(new FileReader(MetisOutput));
                int linecount=1;
                //create temporary bucket
                BufferedWriter bwPart=new BufferedWriter(new FileWriter(file3 + "Parta" + i, true));
                String tuple1="";
                String tuple2="";
                String rdftuple1="";
                String rdftuple2="";
                int rdfsubj3=0;
                int rdfobj3=0;
		String objX = null;
                int rdfobj4=0;
                int rdfsubj4=0;
                int subj1=0;
                int obj1=0;
                int obj2=0;
                int subj2=0;
                //get partition number from Metis output file
                while((fileId=brMain.readLine())!=null){
                    int PartitionNo=Integer.parseInt(fileId);
                    //if partition number is same as bucket we wanted to write into..
                    if(PartitionNo==i)
                    {
                        //end of file NOT reached in PreGraph1 and don't want to skip rows based on logic
                        if(subj1<=linecount && subj1!=-1){
                            //if tuple1 already read
                            if(!tuple1.equals("")){
                                if(i == hmPartition.get(obj1)) {
                                    bwPart.write(tuple1);
                                    bwPart.newLine();
                                }
                            }
                            while((tuple1=brPreG11.readLine())!=null){
                                StringTokenizer strtkn1=new StringTokenizer(tuple1);
                                subj1=Integer.parseInt(strtkn1.nextToken());
                                strtkn1.nextToken();
                                obj1=Integer.parseInt(strtkn1.nextToken());
                                //to be written to same bucket
                                if(linecount == subj1)
                                {
                                    if(i == hmPartition.get(obj1)) {
                                        bwPart.write(tuple1);
                                        bwPart.newLine();
                                    }
                                }
                                //rows that dont belong to same bucket -> skip
                                else if(linecount > subj1)
                                {
                                    continue;
                                }
                                //wait until PreGraph2 is also read
                                else{
                                    break;
                                }
                            }
                            //end of file reached
                            if(tuple1==null){
                                subj1=-1;
                            }
                        }
                        //end of file NOT reached in PreGraph2 and don't want to skip rows based on logic
                        if(obj2<=linecount && obj2!=-1){
                            //if tuple2 already read
                            if(!tuple2.equals("")){
                                if(i == hmPartition.get(subj2)) {
                                    bwPart.write(tuple2);
                                    bwPart.newLine();
                                }
                            }
                            while((tuple2=brPreG22.readLine())!=null){
                                StringTokenizer strtkn2=new StringTokenizer(tuple2);
                                subj2 = Integer.parseInt(strtkn2.nextToken());
                                strtkn2.nextToken();
                                obj2=Integer.parseInt(strtkn2.nextToken());
                                //to be written to same bucket
                                if(linecount == obj2)
                                {
                                    if(i == hmPartition.get(subj2)) {
                                        bwPart.write(tuple2);
                                        bwPart.newLine();
                                    }
                                }
                                //rows that dont belong to same bucket -> skip
                                else if(linecount > obj2)
                                {
                                    continue;
                                }
                                //wait until PreGraph1 is also read
                                else{
                                    break;
                                }
                            }
                            //end of file reached
                            if(tuple2==null){
                                obj2=-1;
                            }
                        }

                        /*if(!Type.equals("T1")) {
                            //code to add tuples from RdfType Graph
                            //end of file NOT reached in PreRdfType1 and don't want to skip rows based on logic
                            if(rdfsubj3<=linecount && rdfsubj3!=-1) {
                                //if tuple1 already read
                                if(!rdftuple1.equals("")){
				    if(objX.contains("\"")) {
					rdfobj3 = MMap.get(objX);
					if(i == hmPartition.get(rdfobj3)) {
					   bwPart.write(rdftuple1);
					   bwPart.newLine();
					}
				    }
				    else {
					rdfobj3 = Integer.parseInt(objX);
                                    	if(i == hmPartition.get(rdfobj3)) {
                                            bwPart.write(rdftuple1);
                                            bwPart.newLine();
                                    	}
 				    }
                                }
                                while((rdftuple1=brPreR11.readLine())!=null){
				    int isub=rdftuple1.indexOf(' ');
		                    rdfsubj3=Integer.parseInt(rdftuple1.substring(0,isub));
                		    String y=rdftuple1.substring(isub+1);
		                    int P=y.indexOf(' ');
		                    String Predicate=y.substring(0,P);
		                    String z=y.substring(P+1);
		                    int Ob=z.lastIndexOf('.');
		                    objX=z.substring(0,Ob-1);

                                    //to be written to same bucket
                                    if(linecount == rdfsubj3)
                                    {
					if(objX.contains("\"")) {
                                       	    rdfobj3 = MMap.get(objX);
                                            if(i == hmPartition.get(rdfobj3)) {
                                         	bwPart.write(rdftuple1);
                                                bwPart.newLine();
                                            }
                                        }
					else {
					    rdfobj3 = Integer.parseInt(objX);
                                            if(i == hmPartition.get(rdfobj3)) {
                                            	bwPart.write(rdftuple1);
                                            	bwPart.newLine();
                                            }
					}
                                    }
                                    //rows that dont belong to same bucket -> skip
                                    else if(linecount > rdfsubj3)
                                    {
                                        continue;
                                    }
                                    //wait until PreGraph2 is also read
                                    else{
                                        break;
                                    }
                                }
                                //end of file reached
                                if(rdftuple1==null){
                                    rdfsubj3=-1;					
                                }
                            }
                            //end of file NOT reached in PreRdfType2 and don't want to skip rows based on logic
                            if(rdfobj4<=linecount && rdfobj4!=-1){
                                //if tuple2 already read
                                if(!rdftuple2.equals("")){
                                    if(i == hmPartition.get(rdfsubj4)) {
                                        bwPart.write(rdftuple2);
                                        bwPart.newLine();
                                    }
                                }
                                while((rdftuple2=brPreR22.readLine())!=null){
                                    int isub=rdftuple2.indexOf(' ');
                                    rdfsubj4=Integer.parseInt(rdftuple2.substring(0,isub));
                                    String y=rdftuple2.substring(isub+1);
                                    int P=y.indexOf(' ');
                                    String Predicate=y.substring(0,P);
                                    String zz=y.substring(P+1);
                                    int Ob=zz.lastIndexOf('.');
                                    String z=zz.substring(0,Ob-1);
 
                                    if(z.contains("\"")) 
	                            	rdfobj4 = MMap.get(z);
				    else
                                    	rdfobj4 = Integer.parseInt(z);
                                    //to be written to same bucket
                                    if(linecount == rdfobj4)
                                    {
                                        if(i == hmPartition.get(rdfsubj4)) {
                                            bwPart.write(rdftuple2);
                                            bwPart.newLine();
                                        }
                                    }
                                    //rows that dont belong to same bucket -> skip
                                    else if(linecount > rdfobj4)
                                    {
                                        continue;
                                    }
                                    //wait until PreGraph1 is also read
                                    else{
                                        break;
                                    }
                                }
                                //end of file reached
                                if(rdftuple2==null){
                                    rdfobj4=-1;
                                }
                            }
                        }*/
                    }
                    //if partition number is NOT same as bucket, so clear the variables already read
                    else {
                        if(linecount==subj1 && linecount==obj2){
                            tuple1="";
                            tuple2="";
                            subj1=0;
                            obj2=0;
                        }
                        //line read in PreGraph1 doesn't belong to this bucket; hence clear it
                        else if(linecount==subj1){
                            tuple1="";
                            subj1=0;
                        }
                        //line read in PreGraph2 doesn't belong to this bucket; hence clear it
                        else if(linecount==obj2){
                            tuple2="";
                            obj2=0;
                        }

                        if(!Type.equals("T1")) {
                            if(linecount==rdfsubj3 && linecount==rdfobj4){
                                rdftuple1="";
                                rdftuple2="";
                                rdfsubj3=0;
                                rdfobj4=0;
                            }
                            //line read in PreGraph1 doesn't belong to this bucket; hence clear it
                            else if(linecount==rdfsubj3){
                                rdftuple1="";
                                rdfsubj3=0;
                            }
                            //line read in PreGraph2 doesn't belong to this bucket; hence clear it
                            else if(linecount==rdfobj4){
                                rdftuple2="";
                                rdfobj4=0;
                            }
                        }
                    }
                    linecount++;
                }
                bwPart.close();
                brMain.close();
            	brPreG11.close();
            	brPreG22.close();
                if(!Type.equals("T1")) {
                    brPreR11.close();
                    brPreR22.close();
                }

                check=true;
            }

            Process p1 = Runtime.getRuntime().exec(file3+"getuniq.sh" + " " + file3+"Parta" + " " + file3+"Part" + " " + NumberOfPartitions);
            p1.waitFor();
	    
	    //close manager
	    recman.close();
	    recman1.close();
        }
        catch(Exception e){
            e.printStackTrace();
            System.out.println("ERROR!!!!");
        }
    }

    public void GetQueue(String DestLoc, String QueueFile, Integer NoOfParts) {
	try{
	    System.out.println("Creating Queue for nHop..\n");
    	    BufferedWriter bw1 = new BufferedWriter(new FileWriter(QueueFile));
	    for(int i=0; i<NoOfParts; i++) {
	    	BufferedReader br1 = new BufferedReader(new FileReader(DestLoc+"Part"+i));
		boolean check1 = false;
		Set<Integer> set = new HashSet<Integer>();
		String trip=null;
		while((trip = br1.readLine()) !=null) {
		    check1 = true;
		    StringTokenizer tkn = new StringTokenizer(trip);
		    int sub=Integer.parseInt(tkn.nextToken());
		    tkn.nextToken();
                    int obj=Integer.parseInt(tkn.nextToken());
			
		    set.add(sub);
		    set.add(obj);
		}
		br1.close();
		
		Iterator it = set.iterator();
		while(it.hasNext()) {
		    bw1.write(it.next() + " ");
		}		
		if(check1) 
		    bw1.newLine();
		set.clear();
	    }
	    bw1.close();	    
	}
	catch(Exception ex) {
	    ex.printStackTrace();
	    System.out.println("Error in GetQueue");
	}
    }

    //method to get list of edgecuts
    public void GetEdgeCuts(String DestLoc, String Type, String Pregraphfile, String MetisOutput, String Metisfile, String queuefile, String GiveEdgeWeights){
        try {
            boolean edges = false;
            if(GiveEdgeWeights.equals("Y"))
                edges = true;
            String x;
            TreeMap<Integer,Integer> PartitionMap = new TreeMap<Integer,Integer>();
            BufferedReader brMP=new BufferedReader(new FileReader(MetisOutput));
            int no=1;
            //which vertex is in which partition
            while((x=brMP.readLine())!=null){
                PartitionMap.put(no, Integer.parseInt(x));
                no++;
            }
            brMP.close();

            int linecount=1;
            BufferedReader brMF = new BufferedReader(new FileReader(Metisfile));
            brMF.readLine();
	    int Total=0;
            int QCount=0;
            TreeMap<Integer, Integer> tm1 = new TreeMap();
            TreeMap<Integer, String> tm2 = new TreeMap();
            while((x=brMF.readLine())!=null) {
                int PartNo = PartitionMap.get(linecount);
                StringTokenizer tkn = new StringTokenizer(x);
                int count=0;
                if(Type.equals("T3") || Type.equals("T4"))
                    tkn.nextToken();
                StringBuilder sb = new StringBuilder();
                while(tkn.hasMoreElements()) {
                    int TempPartitionNo = -1;
                    int VertexNo = Integer.parseInt(tkn.nextToken());
                    if(PartitionMap.containsKey(VertexNo))
                        TempPartitionNo = PartitionMap.get(VertexNo);
                    else
                        continue;
                    if(PartNo != TempPartitionNo){
                        if(count==0) {
                            sb.append(VertexNo);
                            QCount++;
                        }
                        else {
                            sb.append(" ").append(VertexNo);
                        }
                        count++;
                    }
                    if(edges){
                        tkn.nextToken();
                    }
                }
                Total += count;
                if(count>0) {
                    tm1.put(linecount, count);
                    tm2.put(linecount, sb.toString());
                }
                linecount++;
            }
            System.out.println("Total number of edges cuts : " + Total/2);
            //System.out.println("No of Queue Entries: " + QCount);
            brMF.close();

            //reducing the redundancy of traversing heavily connected vertices due to just one edge cut
            Set set = tm1.keySet();
            for(Iterator it=set.iterator();it.hasNext();) {
                int key = (Integer) it.next();
                int val = tm1.get(key);
                if(val == 1) {
                    String st1 = tm2.get(key);
                    int co2 = tm1.get(Integer.parseInt(st1));
                    if (co2 >= 1) {
                        tm1.put(key, 0);
                    }
                }
            }
            BufferedWriter bwEdgeCuts = new BufferedWriter(new FileWriter(queuefile));
            for(Iterator it=set.iterator(); it.hasNext();) {
                int key = (Integer) it.next();
                if(tm1.get(key) != 0) {
                    bwEdgeCuts.write(key+" "+tm2.get(key));
                    bwEdgeCuts.newLine();
                }
                else {
                    tm2.remove(key);
                }
            }
            bwEdgeCuts.close();
            tm1.clear();
            tm2.clear();
        }
        catch (Exception e){
            System.out.println("Error while calculating queuefile");
            e.printStackTrace();
        }

        try{
            BufferedWriter bwEdges = new BufferedWriter(new FileWriter(DestLoc + edgeslist));
            BufferedReader brPreG1 = new BufferedReader(new FileReader(Pregraphfile+"1"));
            BufferedReader brPreG2 = new BufferedReader(new FileReader(Pregraphfile+"2"));
            String edgesNo;
            String triple1 = null;
            String Subj1="";
            String Obj1="";
            String triple2 = null;
            String Subj2="";
            String Obj2="";
            BufferedReader brEdges = new BufferedReader(new FileReader(queuefile));
            while((edgesNo=brEdges.readLine())!=null){
                StringTokenizer tkn = new StringTokenizer(edgesNo);
                String Vertex = tkn.nextToken();
                boolean G1F = true;
                boolean G1w = false;
                boolean t1=true;
                while(tkn.hasMoreElements()) {
                    String OtherVertex = tkn.nextToken();
                    if(G1F) {
                        if(triple1!=null && !G1w) {
                            if(Vertex.equals(Obj1) && OtherVertex.equals(Subj1)) {
                                bwEdges.write(triple1);
                                bwEdges.newLine();
                                continue;
                            }
                            else if(Integer.parseInt(Vertex)<Integer.parseInt(Obj1)) {
                                G1F = false;
                                t1 = false;
                            }
                        }
                        if(t1) {
                        while((triple1=brPreG2.readLine())!=null) {
                            StringTokenizer stkn1 = new StringTokenizer(triple1);
                            Subj1 = stkn1.nextToken();
                            stkn1.nextToken();
                            Obj1 = stkn1.nextToken();
                            if(Vertex.equals(Obj1)) {
                                if(OtherVertex.equals(Subj1)) {
                                    bwEdges.write(triple1);
                                    bwEdges.newLine();
                                    G1w = true;
                                    break;
                                }
                            }
                            else if(Integer.parseInt(Vertex)>Integer.parseInt(Obj1)){
                                continue;
                            }
                            else {
                                G1w = false;
                                G1F = false;
                                break;
                            }
                        }
                        }
                        if(G1w)
                            continue;
                    }

                    if(triple2!=null && !G1w) {
                        if(Vertex.equals(Subj2) && OtherVertex.equals(Obj2)) {
                                bwEdges.write(triple2);
                                bwEdges.newLine();
                                continue;
                            }
                    }
                    while((triple2=brPreG1.readLine())!=null) {
                        StringTokenizer stkn2 = new StringTokenizer(triple2);
                        Subj2 = stkn2.nextToken();
                        stkn2.nextToken();
                        Obj2 = stkn2.nextToken();
                        if(Vertex.equals(Subj2)) {
                            if(OtherVertex.equals(Obj2)) {
                                bwEdges.write(triple2);
                                bwEdges.newLine();
                                G1w = true;
                                break;
                            }
                        }
                        else if(Integer.parseInt(Vertex)>Integer.parseInt(Subj2)) {
                            continue;
                        }
                        else
                            break;
                    }
                }
            }
            brEdges.close();
            brPreG1.close();
            brPreG2.close();
            bwEdges.close();
        }
        catch(Exception ex){
            System.out.println("Error while listing edgecuts");
            ex.printStackTrace();
        }
    }

   //method that inserts unique subjects and objects into TreeMap
    public PrimaryHashMap<Integer,Integer> insertMap(String MetisOutput, RecordManager recman, String Loc){
        PrimaryHashMap<Integer,Integer> map = recman.hashMap(Loc);
        String rec;
        int i=1;
        try{
            BufferedReader bufRdr=new BufferedReader(new FileReader(MetisOutput));
            while((rec=bufRdr.readLine())!=null){
                map.put(i, Integer.parseInt(rec));
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
    public PrimaryTreeMap<String, Integer> insertMap44(String VerticesFile, RecordManager recman, String Loc){
        PrimaryTreeMap<String, Integer> map = recman.treeMap(Loc);
        return map;
    }

    //method to get current time in yyyy-MM-dd HH:mm:ss.SSS format
    public void CurrentTime(){
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        String dateString = sdf.format(new Date());
        System.out.println("CurrentTime = " + dateString);
    }
}
