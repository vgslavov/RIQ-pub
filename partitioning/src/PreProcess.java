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
public class PreProcess {

    public static void main(String[] args) {
        // TODO code application logic here
        if(args.length!=7){
            System.out.println("Invalid Command line arguments!\n");
            System.out.println("Arguments: <DestinationFilePath> <connectedcomps> <Vertices> <OutputFile> <PredicatesFile> <VerticesCount> <EdgeCuts>");
        }
        else{
            PreProcess drObject=new PreProcess();
            try{
                System.out.println("PreProcess.java ---> Execution started!");
                //get the time when actual program execution starts
                drObject.CurrentTime();
                //core method
                System.out.println();
//                System.out.println("Adding RdfTypes..");
//                drObject.GetRdfType(args[0], args[1], args[0]+"AddRdfType");
 		Process p3=Runtime.getRuntime().exec("mkdir "+args[0]+"Databases/13");
            p3.waitFor();

                System.out.println("In Conversion..");
                RecordManager recman3 = RecordManagerFactory.createRecordManager(args[0]+"Databases/13/hugedata1");
                PrimaryHashMap<Integer, String> hmvertices = drObject.insertMap(args[2], recman3, args[0]+"Databases/13/hugemap1");
                HashMap<Integer, String> hmTemp = new HashMap<Integer, String>();
                BufferedReader brPred = new BufferedReader(new FileReader(args[4]));
                String Pred = null;
                int next = Integer.parseInt(args[5]) + 1;
                while((Pred = brPred.readLine()) != null) {
                    hmTemp.put(next, Pred);
                    next++;
                }
                drObject.WriteLiterals(args[1], args[3], hmvertices, hmTemp);
		drObject.WriteLiterals(args[0]+"edgesID", args[6], hmvertices, hmTemp);
                recman3.close();
                //get the time when execution ends
                drObject.CurrentTime();
                System.out.println("PreProcess.java ---> Execution completed!");
            }
            catch(Exception e){
                System.out.println("Error in WriteLiterals!");
                e.printStackTrace();
            }
        }
    }

    public void GetRdfType(String arg1, String File1, String File2) {
        try {
            Process p3=Runtime.getRuntime().exec("mkdir "+arg1+"Databases/11 " +arg1+"Databases/12 " + arg1+"Databases/13");
            p3.waitFor();
            System.out.println("In GetRdfType..");
            RecordManager recman1 = RecordManagerFactory.createRecordManager(arg1+"Databases/11/hugedata1");
            PrimaryHashMap<Integer,TreeMap<String, Integer>> hm = GetRdfTypeMap(arg1+"Databases/11/hugemap1", arg1+"PreRdfType", recman1);
            RecordManager recman2 = RecordManagerFactory.createRecordManager(arg1+"Databases/12/hugedata1");
            PrimaryTreeMap<String, Boolean> tmVisited = recman2.treeMap(arg1+"Databases/12/hugemap1");
            BufferedReader brMain = new BufferedReader(new FileReader(File1));
            BufferedWriter bwMain = new BufferedWriter(new FileWriter(File2));
            String triple;
            int count = 0;	

            while((triple=brMain.readLine())!=null) {
                if(triple.equals("")) {
                    tmVisited.clear();
                    bwMain.newLine();
                }
                else {
                    StringTokenizer strtkn = new StringTokenizer(triple);
                    int sub = Integer.parseInt(strtkn.nextToken());
                    strtkn.nextToken();
                    int obj = Integer.parseInt(strtkn.nextToken());
                    bwMain.write(triple);
                    bwMain.newLine();
                    if(hm.containsKey(sub)) {
                        TreeMap<String, Integer> tm = hm.get(sub);
                        Set set = tm.keySet();
                        for(Iterator it=set.iterator();it.hasNext();) {
                            String key = (String) it.next();
                            int i=key.lastIndexOf(' ');
                            String node = key.substring(0,i);
                            int pred = Integer.parseInt(key.substring(i+1));
                            if(node.contains("\"")) {
                                String trp = sub + " " + pred + " " + node + " .";
				if(!tmVisited.containsKey(trp)) {
                                   bwMain.write(trp);
                                   bwMain.newLine();
                                   tmVisited.put(trp, true);
                                   count++;
			        }
                            }
                            else {
                                int n = Integer.parseInt(node);
                                if(n > 0) {
                                    String trp = sub + " " + pred + " " + n + " .";
                                    if(!tmVisited.containsKey(trp)) {
                                        bwMain.write(trp);
                                        bwMain.newLine();
                                        tmVisited.put(trp, true);
                                        count++;
                                    }
                                }
                                else {
                                    String trp = (n*-1) + " " + pred + " " + sub + " .";
                                    if(!tmVisited.containsKey(trp)) {
                                        bwMain.write(trp);
                                        bwMain.newLine();
                                        tmVisited.put(trp, true);
                                        count++;
                                    }
                                }
                            }
			    if(count%10000==0) {
				recman2.commit();
			    }
                        }
                    }
                    if(hm.containsKey(obj)) {
                        TreeMap<String, Integer> tm = hm.get(obj);
                        Set set = tm.keySet();
                        for(Iterator it=set.iterator();it.hasNext();) {
                            String key = (String) it.next();
                            int i=key.lastIndexOf(' ');
                            String node = key.substring(0,i);
                            int pred = Integer.parseInt(key.substring(i+1));
                            if(node.contains("\"")) {
                                String trp = obj + " " + pred + " " + node + " .";
			   	if(!tmVisited.containsKey(trp)) {
                                   bwMain.write(trp);
                                   bwMain.newLine();
                                   tmVisited.put(trp, true);
                                   count++;
				}
                            }
                            else {
                                int n = Integer.parseInt(node);
                                if(n > 0) {
                                    String trp = obj + " " + pred + " " + n + " .";
                                    if(!tmVisited.containsKey(trp)) {
                                        bwMain.write(trp);
                                        bwMain.newLine();
                                        tmVisited.put(trp, true);
                                        count++;
                                    }
                                }
                                else {
                                    String trp = (n*-1) + " " + pred + " " + obj + " .";
                                    if(!tmVisited.containsKey(trp)) {
                                        bwMain.write(trp);
                                        bwMain.newLine();
                                        tmVisited.put(trp, true);
                                        count++;
                                    }
                                }
                            }
			    if(count%10000==0) {
				recman2.commit();
			    }
                        }
                    }
                    if(count%10000==0 || count%10001==0) {
                        recman1.commit();
                        recman2.commit();
                    }
                }
            }
	    hm.clear();
	    bwMain.close();
	    brMain.close();
            recman1.close();
            recman2.close();
            System.out.println("Finished GetRdfType..");
        }
        catch(Exception ex) {
            System.out.println("error in GetRdfType");
            ex.printStackTrace();
        }
    }

     //replace IDs with corresponding subjects and objects
    public void WriteLiterals(String DestLocReader, String DestLocWriter, PrimaryHashMap<Integer,String> map, HashMap<Integer, String> hmTemp){
        try{
            String triple;
	    int count = 0;
            System.out.println("Writing literals..");
            BufferedWriter bwOut = new BufferedWriter(new FileWriter(DestLocWriter));
            BufferedReader brRead = new BufferedReader(new FileReader(DestLocReader));
            while((triple=brRead.readLine())!=null){
                if(triple.equals("")) {
                    bwOut.newLine();
	            count++;
		    System.out.println("Processed component: " + count);
                }
                else {
                    StringBuilder OrgTriple = new StringBuilder();

                    int i=triple.indexOf(' ');
                    //subject
                    String Val=(String) map.get(Integer.parseInt(triple.substring(0,i)));
                    OrgTriple.append(Val).append(" ");

                    String y=triple.substring(i+1);
                    int P=y.indexOf(' ');

                    //if predicate
                    Val=(String) hmTemp.get(Integer.parseInt(y.substring(0,P)));
                    OrgTriple.append(Val).append(" ");

                    String z=y.substring(P+1);
                    int Ob=z.lastIndexOf('.');

                    //if object
                    String obj=z.substring(0,Ob-1);
                    if(obj.contains("\"")) {
                        OrgTriple.append(obj).append(" .");
                    }
                    else {
                        String temp=(String) map.get(Integer.parseInt(obj));
                        OrgTriple.append(temp).append(" .");
                    }
                    bwOut.write(OrgTriple.toString());
                    bwOut.newLine();
                }
            }
            brRead.close();
            bwOut.close();
        }
        catch(Exception ex){
            System.out.println("error in WriterLiterals");
            ex.printStackTrace();
        }
    }

    public PrimaryHashMap<Integer,TreeMap<String, Integer>> GetRdfTypeMap(String DestLoc, String PreRdfType, RecordManager recman1) {
        try {
            System.out.println("In GetRdfTypeMap..");
            PrimaryHashMap<Integer,TreeMap<String, Integer>> tmRdf = recman1.hashMap(DestLoc);
            BufferedReader brPreR11 = new BufferedReader(new FileReader(PreRdfType+"1"));
            int count=0;
            String Triple=brPreR11.readLine();
            int i=Triple.indexOf(' ');
            int Prv = Integer.parseInt(Triple.substring(0,i));
            String y=Triple.substring(i+1);
            int P=y.indexOf(' ');
            String Pred=y.substring(0,P);
            String z=y.substring(P+1);
            int Ob=z.lastIndexOf('.');
            String Obj=z.substring(0,Ob-1);
            TreeMap<String, Integer> tempMap = new TreeMap();
            tempMap.put(Obj+ " " + Pred, 1);
            //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
            while((Triple = brPreR11.readLine()) != null){
                i=Triple.indexOf(' ');
                int Pst = Integer.parseInt(Triple.substring(0,i));
                y=Triple.substring(i+1);
                P=y.indexOf(' ');
                Pred=y.substring(0,P);
                z=y.substring(P+1);
                Ob=z.lastIndexOf('.');
                Obj=z.substring(0,Ob-1);
                if(Pst == Prv){
                    tempMap.put(Obj + " " + Pred, 1);
                }
                else{
                    count++;
                    tmRdf.put(Prv, tempMap);
                    Prv = Pst;
                    tempMap = new TreeMap();
                    tempMap.put(Obj +" " +Pred, 1);
                }
                if(count % 100000 == 0)
                {
                    recman1.commit();
                }
            }
            tmRdf.put(Prv, tempMap);
            recman1.commit();
            brPreR11.close();
 
            Integer Subj;
            tempMap = new TreeMap();
            BufferedReader brPreR22 = new BufferedReader(new FileReader(PreRdfType+"2"));
            while((Triple=brPreR22.readLine())!=null) {
                i=Triple.indexOf(' ');
                Subj = Integer.parseInt(Triple.substring(0,i));
                y=Triple.substring(i+1);
                P=y.indexOf(' ');
                Pred=y.substring(0,P);
                z=y.substring(P+1);
                Ob=z.lastIndexOf('.');
                Obj=z.substring(0,Ob-1);

                if(Obj.startsWith("\""))
                    continue;
                else{
                    Prv = Integer.parseInt(Obj);
                    if(tmRdf.containsKey(Prv))
                        tempMap = tmRdf.get(Prv);
                    else {
                        tempMap = new TreeMap();
		    }
                    tempMap.put(-1*Subj+ " " + Pred, 1);
                    break;
                }
            }
            count=0;
            //Run until triples exist in file2(complete file) and it's subject is same as subject in file1(unique subject list)
            while((Triple = brPreR22.readLine()) != null){
                i=Triple.indexOf(' ');
                Subj = Integer.parseInt(Triple.substring(0,i));
                y=Triple.substring(i+1);
                P=y.indexOf(' ');
                Pred=y.substring(0,P);
                z=y.substring(P+1);
                Ob=z.lastIndexOf('.');
                int Pst = Integer.parseInt(z.substring(0,Ob-1));

                if(Pst == Prv){
                    tempMap.put(-1*Subj+ " " + Pred, 1);
                }
                else{
                    count++;
                    tmRdf.put(Prv, tempMap);
                    Prv = Pst;
                    if(tmRdf.containsKey(Prv))
                        tempMap = tmRdf.get(Prv);
                    else {
                        tempMap = new TreeMap();
		    }
                    tempMap.put(-1*Subj+ " " + Pred, 1);
                }
                if(count % 100000 == 0)
                {
                    recman1.commit();
                }
            }
            tmRdf.put(Prv, tempMap);
            recman1.commit();
            brPreR22.close();
            System.out.println("Finished GetRdfTypeMap..");
            return tmRdf;
        }
        catch(Exception ex) {
            System.out.println("error in GetRdfTypeMap!");
            ex.printStackTrace();
        }
        return null;
    }

    //method that inserts unique subjects and objects into TreeMap
    public PrimaryHashMap<Integer, String> insertMap(String VerticesFile, RecordManager recman, String Loc){
        PrimaryHashMap<Integer, String> map = recman.hashMap(Loc);
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

