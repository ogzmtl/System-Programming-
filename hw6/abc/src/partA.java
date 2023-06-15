import java.io.File;
import java.io.FileNotFoundException;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Scanner;
import java.util.Stack;
import javax.swing.tree.DefaultMutableTreeNode;

public class partA{
    
    private String[][] txtToArray = new String[1][];
    private DefaultMutableTreeNode tree = new DefaultMutableTreeNode("Root");
    private int stepCounter = 0;
    private DefaultMutableTreeNode nn = new DefaultMutableTreeNode();

    /**
     * reads from file and keeps file structure in 2d dynamic array structure
     * @param filename given filenmae which is always tree.txt
     * @throws FileNotFoundException if file is not exist sends this exception
     */
    public void readFromTxt(String filename) throws FileNotFoundException{
        File file = new File(filename);
        String line;
        int counter = 0; 
        String[] splitted; 

        try (Scanner scanner = new Scanner(file)) {
            while(scanner.hasNextLine())
            {
                line = scanner.nextLine();
                splitted = line.split(";");

                if(txtToArray[counter] == null)
                {
                    txtToArray[counter] = new String[splitted.length];
                }

                for(int i = 0; i < splitted.length; i++){
                    txtToArray[counter][i] = splitted[i];
                }
                if(counter == txtToArray.length-1 && (scanner.hasNextLine())){
                    txtToArray = reallocate(counter+1);
                }  
                counter++;           
            }
            // for(int i = 0; i < txtToArray.length; i++)
            // {
            //     for(int j = 0; j < txtToArray[i].length; j++){
            //         System.out.print(txtToArray[i][j] + " ");
            //     }
            //     System.out.print("\n");
            // }
        }
    }

    /**
     * reallocate the 2d string array in every one elements addition
     * @param counter counter keeps the old size of the array 
     * @return returns new copied 2d array to original array
     */
    private String[][] reallocate(int counter){
        String[][] tempString = new String[1+counter][];

        for(int i = 0; i < txtToArray.length; i++)
        {
            if(tempString[i] == null ){
                tempString[i] = new String[txtToArray[i].length];
            }
            for(int j = 0; j < tempString[i].length; j++)
            {
                tempString[i][j] = txtToArray[i][j];
            }
        }
        return tempString;
    }

    /**
     * creates tree from the given 2d string 
     * this function use iterative method 
     * first we create root of the node 
     * behind its idea is, first we create a root node of the tree which is root 
     * then this function check first row of the array in every splitted character
     *  node.add() method invokes by method after first column was written
     *  other columns writtten to that node I move root reference to the child node
     *  and after this reference move iteration (we assign root = child ) and 
     * then we invoke add function on the root node. 
     * It adds every column by root iteration with help of this function 
     */
    public void tree(){
        DefaultMutableTreeNode temp = tree;

        for(int i = 0; i < txtToArray.length; i++){

            temp = tree;
            for(int j = 0; j < txtToArray[i].length; j++){

                DefaultMutableTreeNode lecture = new DefaultMutableTreeNode(txtToArray[i][j]);

                if(insertToTree(temp, lecture)){
                    temp.add(lecture);
                    temp = lecture;
                }
                else{
                    temp = moveTreeObject(temp, lecture);
                }   
            }
            // System.out.println("------------");
        }
        // insertToTree(tree, null);
    }
/**
 * this function looks if given node is insertable to the tree, tree already 
 * include given node data it must skip that node this method check that situation
 *  with helper isInclude method 
 * @param tree root of the tree
 * @param node data node which is compared to every node of root
 * @return if is not included in the tree then return false
 */
    public boolean insertToTree(DefaultMutableTreeNode tree,DefaultMutableTreeNode node){
        if(tree.isLeaf()){
        // System.out.println("LeafinsertToTree " + tree.getUserObject() + " node: " + node.getUserObject()); 
            return true;
        }
        else{
            if(!isInclude(tree, node)){
                // System.out.println("insertToTree " + tree.getUserObject() + " node: " + node.getUserObject()); 
                return true;
            }

        }
        // System.out.println("false");
        return false;
    }
    
    /**
     * helper function of the insertToTree method checks node is included in the tree
     * or not 
    * @param tree root of the tree
    * @param node data node which is compared to every node of root
    * @return if is not included in the tree then return false
     */
    private boolean isInclude(DefaultMutableTreeNode tree,DefaultMutableTreeNode node){
        if (tree == null || node == null) {
            return false;
        }   
        int childCount = tree.getChildCount();
        // System.out.println("tree : " + tree.getUserObject() +" "+childCount);
        for(int i=0;i<childCount;i++){
     
            DefaultMutableTreeNode child = (DefaultMutableTreeNode) tree.getChildAt(i);
            if(child == null) {
                return true;
            }
            if(node.getUserObject().equals(child.getUserObject())){
                return true;
            }
        }
        return false;
    }

    /**
     * if new node is already added to the tree then this function iterates 
     * through the children to reach the data node inside the tree
     * @param tree root of the tree 
     * @param node data node to insert or iteratively move
     * @return if data node object found in the tree returns founded node reference
     */
    private DefaultMutableTreeNode moveTreeObject(DefaultMutableTreeNode tree,DefaultMutableTreeNode node){

        int childCount = tree.getChildCount();
        // System.out.println("tree : " + tree.getUserObject() +" "+childCount);
        for(int i=0;i<childCount;i++){
     
            DefaultMutableTreeNode child = (DefaultMutableTreeNode) tree.getChildAt(i);

            if(node.getUserObject().equals(child.getUserObject())){
                return child;
            }
        }
        return null;
    }

    /**
     * I use queue data structure in this method 
     * First I accessed the child nodes starting from root, then I wrote a for loop 
     * that will visit each child nodes of root first. Within this for loop, I assigned 
     * child node I visited to a queue, then I visited the 2nd and 3rd child nodes if exists, 
     * starting with the first child I get childs child nodes , and assigned these values to the 
     * queue in that order. While doing these, at the same time, 
     * I checked whether the value I was looking for and the value
     * I would assign to the queue at the moment matched, and if the result was correct,
     * I finished the process.
     * @param userInput given String value which is from system input
     * @return if process is done successfully then return true otherwise return false
     */
    public boolean BFS(String userInput){
        
        DefaultMutableTreeNode temp =(DefaultMutableTreeNode) tree.getRoot();
        Queue<DefaultMutableTreeNode>queueNode = new LinkedList<DefaultMutableTreeNode>();
        System.out.println("Using BFS to find "+ userInput + " in the tree...");

        queueNode.add((DefaultMutableTreeNode)temp.getRoot());
        int counter = 1; 
        while(!queueNode.isEmpty())
        {
            DefaultMutableTreeNode printedNode = (DefaultMutableTreeNode) queueNode.poll();
            
            if(printedNode.getUserObject().equals(userInput))
            {
                System.out.println("Step "+ (counter++) +" -> " +printedNode + "(Found!)");
                return true;
            }
            else{
                System.out.println("Step "+ (counter++) +" -> " +printedNode);
            }
            int childCount = printedNode.getChildCount();
            if(childCount != 0)
            {
                for(int i = 0; i < childCount; i++)
                {
                    DefaultMutableTreeNode childNode = (DefaultMutableTreeNode) printedNode.getChildAt(i);
                    queueNode.add(childNode);
                }
            }
        }
        System.out.println("Not found.");
        return false;
    }
/**
 * Recursive Depth-Firsth Search method with helper function
 * @param userInput given input
 */
    public void DFSRecursion(String userInput)
    {
        if(!helperDFSRecursion(userInput, tree)){
            System.out.println("Not found.");
        }
    }
    /**
     * Searches the tree using the Depth-First Search algorithm with a stack.
     * Starting from the root node, the method traverses the tree by visiting
     * each node's children first before moving on to the next sibling. 
     * The method stops the traversal and returns true when it finds a node 
     * with user object equal to the given input. If no such node is found,
     * the method returns false.
     *
     * @param userInput the string to search for in the tree
     * @return true if a node with the given user object is found, false otherwise
     */
    public boolean DFSStack(String userInput)   
    {
        Stack<DefaultMutableTreeNode> stackNode = new Stack<DefaultMutableTreeNode>();
        DefaultMutableTreeNode temp = tree; 

        int counter =0; 
        stackNode.push(temp);

        while(!stackNode.isEmpty())
        {
            counter++;
            DefaultMutableTreeNode node = stackNode.pop();
            
            if(node.getUserObject().equals(userInput))
            {
                System.out.println("Step " + counter +" -> " + node.getUserObject() + "(Found!)");
                return true;
            }
            else {
                System.out.println("Step " + counter +" -> " + node.getUserObject());
            }

            int childCount = node.getChildCount();

            for(int i = 0; i < childCount; i++) {
                stackNode.push((DefaultMutableTreeNode)node.getChildAt(i));
            }
        }
        System.out.println("Not found.");
        return false;
    }
    
    /**
     * Recursively searches the subtree rooted at the given node using
     * the Depth-First Search algorithm. The method visits each node in the
     * subtree by first exploring the leftmost branch until it reaches a 
     * leaf node, then backtracks and explores the next unvisited branch. 
     * The method stops the traversal and returns true when it finds a node 
     * with user object equal to the given input. If no such node is found,
     * the method returns false.
     *
     * @param userInput the string to search for in the subtree
     * @param node the root node of the subtree to search in
     * @return true if a node with the given user object is found, false otherwise
     */
    private boolean helperDFSRecursion(String userInput, DefaultMutableTreeNode node)
    {
        stepCounter++;
        if(node == null){
            return false;
        } 
        
        if(node.getUserObject().equals(userInput)){
            System.out.println("Step "+ (stepCounter) +" -> " +node + "(Found!)");
            return true;
        }
        else{
            System.out.println("Step "+ (stepCounter) +" -> " +node);
        }
            
        int childCount = node.getChildCount(); 
        if(childCount != 0){   
            for(int i = childCount-1; i >= 0; i--){
                DefaultMutableTreeNode childNode = (DefaultMutableTreeNode) node.getChildAt(i);
                if(helperDFSRecursion(userInput, childNode)){
                    return true;
                }
            }
        }
        return false;
    }
/**
 * Recursive postOrderTraversal method with helper function
 * @param userInput given input
 */
    public void PostOrderTraversal(String userInput){
        stepCounter = 0;
        if(!helperPostOrderTraversal(userInput, tree)){
            System.out.println("Not found.");
        }

    }
    /**
    * Recursively traverses the tree using post-order traversal and searches for the given user input.
    * The method starts by visiting the leftmost child, then its siblings, and then the parent.
    * If the given user input is found in the tree, it prints the steps taken and returns true.
    * If the given user input is not found in the tree, it prints the steps taken and returns false.
    @param userInput The user input to be searched in the tree.
    @param node The current node being visited during the traversal.
    @return true if the given user input is found in the tree, false otherwise.
    */
    private boolean helperPostOrderTraversal(String userInput, DefaultMutableTreeNode node ){
        
        int childCount = node.getChildCount();
        if(childCount != 0 ){
            for(int i = 0; i < childCount; i++)
            {
                if(helperPostOrderTraversal(userInput, (DefaultMutableTreeNode)node.getChildAt(i)))
                {
                    return true;
                }
            }
        }
        stepCounter++;
        if(node.getUserObject().equals(userInput)){
            System.out.println("Step "+ (stepCounter) +" -> " +node + "(Found!)");
            return true;
        }
        else{
            System.out.println("Step "+ (stepCounter) +" -> " +node);
            return false;
        }       
        
    }
/**
 * move method includes 4 main methods inside generally this method use as an interface method
 * @param source source string from the user input 
 * @param destination  destination string from the user input which can be only one string value
 */
    public void move(String source, String destination)
    {
        // DefaultMutableTreeNode sourceNode = isExists(source); 
        // DefaultMutableTreeNode destinationNode = isExists(destination);
    
        // System.out.println(sourceNode);
        if(createNode(tree, source) == null){
            return;
        }

        // System.out.println("aaaa" + nn);
        // System.out.println("aaaa" + nn.getChildAt(0) + "bbbb" + nn.getChildAt(0));
        add(nn, destination, source);

        cleanTree(tree);
        // remove(tree);
        
        // add(destinationNode)

        // if(addToDest(destination)){
        //     System.out.println("true");
        // }
        //remove new queue;
    }

    /**
     * This tree just checks the roots child nodes 
     * to delete year which has not any child node
     * @param root tree root node
     */
    private void cleanTree(DefaultMutableTreeNode root)
    {

        for(int i = 0; i < root.getChildCount(); i++)
        {
            int childsChild = root.getChildAt(i).getChildCount();
            if(childsChild == 0)
            {
                root.remove((DefaultMutableTreeNode) root.getChildAt(i));

            }
        }
    }
/**
 * Iterates the root and moves the createNode() functions to the destination path 
 * Overwritten issues by processing with the counter variable
 * @param sourceNode given source path with the help of createNode() method
 * @param dest destination path 
 * @param src source string value 
 */
    public void add(DefaultMutableTreeNode sourceNode, String dest, String src)
    {
        DefaultMutableTreeNode temp = tree;
        String[] splitted = src.split(",");
        int counter = 0;
        int counter2 = 0;
        temp = iterateRoot(temp, dest);
        // System.out.println(temp.getChildAt(0));
        for(int i = 0; i < sourceNode.getChildCount(); i++)
        {
            int childCount = temp.getChildCount();
            // System.out.println(temp.getChildCount());
            for(int j = 0; j < childCount; j++)
            {
                // System.out.println(temp);
                if(((DefaultMutableTreeNode) temp.getChildAt(j)).getUserObject().equals(((DefaultMutableTreeNode) sourceNode.getChildAt(0)).getUserObject()))
                {
                    temp = (DefaultMutableTreeNode) temp.getChildAt(j);
                    sourceNode = (DefaultMutableTreeNode) sourceNode.getChildAt(0);
                    counter++;
                    break;
                }
            }
            counter2++;
        }
        if(counter == counter2 )
        {
            String message = "";
            System.out.print("Moved ");
            for(int k = 0; k < splitted.length; k++){
                
                if(k != splitted.length-1) message += ( splitted[k] + "->");
                else 
                {
                    message += splitted[k];
                    System.out.println( message +" to " + dest);
                }
            }
            System.out.println(message + " has been overwritten.");
        }
        else{
            temp.add((DefaultMutableTreeNode)sourceNode.getChildAt(0));
        }
        

    }
    /**

    * Iterates over a given root node to find a destination node represented by a string of comma-separated values.
    * If the destination node is found, it returns the node. Otherwise, it returns null.
    * If the destination node is not found, it inserts a new node to the tree
    * but destination node must be year this method this add operation
    @param root the root node of the tree to search for the destination node
    @param dest the destination node represented by a string of comma-separated values
    @return the destination node if found, otherwise null
    */
    private DefaultMutableTreeNode iterateRoot(DefaultMutableTreeNode root, String dest)
    {
        String[] splitted = dest.split(",");
        int counter = 0;
        // System.out.println(splitted.length);

        
        for(int i =0; i < splitted.length; i++) 
        {
            int childCount = root.getChildCount();
            DefaultMutableTreeNode compare = new DefaultMutableTreeNode(splitted[i]);
            for(int j = 0; j < childCount; j++)
            {
                // System.out.println(dest)
                // System.out.println(splitted[i]);
                if(((DefaultMutableTreeNode)root.getChildAt(j)).getUserObject().equals(compare.getUserObject())){
                    root = (DefaultMutableTreeNode)root.getChildAt(j);
                    counter++;
                    break;
                }
            } 
            
            if(i == 0 && 1 != counter)
            {
                // System.out.println(splitted[i]);
                if(insertToTree(tree, new DefaultMutableTreeNode(splitted[i]))){
                    DefaultMutableTreeNode childNew = new DefaultMutableTreeNode(splitted[i]);
                    root.add(childNew);
                    // System.out.println(root.getChildCount());
                    counter++;
                    root = childNew;
                }
                break;
            }
        }
        if(counter == splitted.length){
            return root;
        }
        else{
            return null;
        }
    }
    /**
    * Creates and returns a new node for the given source path in the actual tree.
    @param root The root node of the tree.
    @param source A string representing the path to be created in the tree.
    @return The created node in the tree or null if any of the nodes in the path don't exist.
    */
    private DefaultMutableTreeNode createNode(DefaultMutableTreeNode root, String source)
    {
        DefaultMutableTreeNode temp = root;
        DefaultMutableTreeNode nnTemp = nn;
        String[] splitted = source.split(",");
        int counter = 0;
        
        for(int i =0; i < splitted.length; i++) 
        {
            int childCount = temp.getChildCount();
            for(int j = 0; j < childCount; j++)
            {
                DefaultMutableTreeNode childNode = (DefaultMutableTreeNode) temp.getChildAt(j);
                // System.out.println(childNode);
                if(childNode.getUserObject().equals(splitted[i]))
                {
                    counter++;
                    if(i == splitted.length -1)
                    {
                        if(nnTemp == null)
                        {
                            nnTemp = new DefaultMutableTreeNode(((DefaultMutableTreeNode) temp.getChildAt(j)).getUserObject());
                        }
                        else{
                            nnTemp.add(childNode);
                        }                        
                    }
                    else if(i != 0){
                        // if(nnTemp == null)
                        // {
                        //     nnTemp = new DefaultMutableTreeNode(((DefaultMutableTreeNode) temp.getChildAt(j)).getUserObject());
                        // }
                        // else
                        // {
                            DefaultMutableTreeNode newNode = new DefaultMutableTreeNode(((DefaultMutableTreeNode) temp.getChildAt(j)).getUserObject());
                            nnTemp.add(newNode);
                            nnTemp = (DefaultMutableTreeNode)nnTemp.getChildAt(0);
                    //     }
                    //     // System.out.println("ovvvvv"+ nn);
                    //     temp = childNode;
                        temp = childNode;
                    }
                    else if(i == 0)
                    {
                        temp = childNode;
                    }
                    // else{
                        
                    // }
                    
                    break;
                }
            } 
        }
        if(counter != splitted.length ){
            System.out.print("Cannot move ");
            for(int k = 0; k < splitted.length; k++){
                
                if(k != splitted.length-1) System.out.print( splitted[k] + " -> ");
                else 
                {
                    System.out.print( splitted[k] + " because it doesn't exist in the tree.\n");
                    return null;
                }
            }

        }
        return temp;
    }

    /**
     * getter for tree 
     * @return tree
     */
    public DefaultMutableTreeNode getTree()
    {
        return tree;
    }

}

