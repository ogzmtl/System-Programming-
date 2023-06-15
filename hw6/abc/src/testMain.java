import java.io.FileNotFoundException;
import java.util.Scanner;
import javax.swing.*;

public class testMain {
    public static void main(String[] args) throws FileNotFoundException {
        
        String filename = new String("tree.txt");
        partA first = new partA(); 
        int userInput = 0;

        JFrame fa;  
              
        first.readFromTxt(filename);
        first.tree(); 
        Scanner scanner = new Scanner(System.in);
        Scanner bfsScanner = new Scanner(System.in);
        while(userInput != -1)
        {
            System.out.println("-------HW 5------");
            System.out.println("-----------------");
            System.out.println("1.Show the tree");
            System.out.println("2.BFS");
            System.out.println("3.DFS");
            System.out.println("4.PostOrderTraversal");
            System.out.println("5.Move");
            System.out.println("6.Exit");
                String newUserInput = new String();
            try{ 
                userInput = scanner.nextInt();   
                       
                switch(userInput)
                {

                    case 1:
                        fa = new JFrame();
                        JTree jt=new JTree(first.getTree());
                        fa.add(jt);  
                        fa.setSize(200,200);  
                        fa.setVisible(true);  
                        break; 
                    case 2:
                       
                        System.out.println("Enter an input to search with BFS.");
                        
                        newUserInput = bfsScanner.nextLine();
                        first.BFS(newUserInput);
                        break;


                    case 3:
                        System.out.println("Enter an input to search with DFS.");
                        newUserInput = bfsScanner.nextLine();
                        first.DFSStack(newUserInput);
                        break;
                    
                    case 4:
                        System.out.println("Enter an input to search with Post Order Traversal.");
                        newUserInput = bfsScanner.nextLine();
                        first.PostOrderTraversal(newUserInput);
                        break;
                    
                    case 5:
                        System.out.println("Enter source of object (at least 2 source and divided by comma) : " );
                        String source = bfsScanner.nextLine();
                        System.out.println("Enter destination of object : (year target)" );
                        String target = bfsScanner.nextLine();
                        if(isCorrectInput(source, target)){
                            first.move(source, target);
                        }
                        break;
                    case 6:
                        userInput = -1;
                        System.out.println("BYE");
                        System.out.println("----------------");
                        break;

                    default:
                        System.out.println("Invalid Entry Program Terminates ");
                        userInput = -1;
                }
            }
            catch(Exception e ){
                System.out.println("Invalid Entry Program Terminates");
                userInput = -1;
            }
    }
        bfsScanner.close();
        scanner.close();
    }

    private static boolean isCorrectInput(String source, String target) throws Exception {
        String[] splitted = source.split(",");

        if(splitted.length >= 2 )
        {
            if(target == null){
                return false;
            }
    
            try{
                double d = Double.parseDouble(target);
            }catch(Exception e){
                throw e;
            }
            return true;
        }
        else{
            throw new Exception();
        }


    }
}
