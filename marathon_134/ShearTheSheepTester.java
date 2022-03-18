import java.awt.*;
import java.awt.geom.*;
import java.util.*;
import java.io.*;
import javax.imageio.*;
import java.util.List;
import java.util.ArrayList;
import java.awt.image.BufferedImage;

import com.topcoder.marathon.*;

public class ShearTheSheepTester extends MarathonAnimatedVis {
    //parameter ranges
    private static final int minN = 10, maxN = 30; // grid size range
    private static final int minB = 1, maxB = 8; // number of barns range
    private static final int minC = 3, maxC = 21; // max wool carrying capacity range
    private static final double minRF = 0.1, maxRF = 0.3; // farmer ratio for each sheep
    private static final double minPS = 0.01, maxPS = 0.1; // sheep probability range
    private static final double minPW = 0.02, maxPW = 0.15; // wool growing probability range
    private static final double minT = 0.05, maxT = 0.2; // tree probability range

    //Inputs
    private int N;            //grid size
    private int B;            //number of barns
    private int C;            //max wool carrying capacity
    private double sheepP;    //sheep probability
    private double farmerR;   //farmers for each sheep
    private double treeP;     //tree probability    
    private double[] growthP; //wool growth probability for each sheep
    
    //Constants
    private static final char Barn = 'X';
    private static final char Tree = '#';
    private static final char Farmer = 'a'; // up to 'a'+C
    private static final char Sheep = 'A';  // from 'A'
    private static final char SheepFull = 'D';  // up to 'D' (3 layers of wool, B,C,D)
    private static final char Empty = '.';
    private static final int[] dr = {0,  -1,  0,  1, 0};
    private static final int[] dc = {-1,  0,  1,  0, 0};
    private static final int[] woolx = {40,48,54,40,26,32,40,63,54,26,17,75,83,89,75,61,67,75,98,89,61,52};
    private static final int[] wooly = {80,92,76,66,76,92,105,87,60,60,87,80,92,76,66,76,92,105,87,60,60,87};
    private static final int MaxTurns = 1000; // Number of turns for each case

    //Derived values
    private int numFarmers;
    private int numSheep;
    private char FarmerFull;
    private int woolOnSheep;
    private int woolWithFarmers;
    private int[][] woolGrow;    // wool growth for each sheep for each turn
    private int[][] sheepMoves;  // sheep movement for each turn
    
    //Graphics
    private Image[] farmerPic;
    private Image[] sheepPic;
    private Image barnPic;
    private Image treePic;

    //State Control
    private char[][] grid; 
    private int[][] sheepIndex; // Keep track of which sheep on which cell
    private int numTurns;
    private int score;


    protected boolean isFarmersSheepConnected()
    {
      boolean[][] visited = new boolean[N][N];
      List<Integer> qr = new ArrayList<>();
      List<Integer> qc = new ArrayList<>();
      // Find a farmer
      for (int rc=0;rc<N*N;rc++)
      {
        int r = rc/N;
        int c = rc%N;
        if (grid[r][c]==Farmer)
        {
          // Fill the grid from the farmer, it should reach all sheep and other farmers and barns
          qr.add(r);
          qc.add(c);
          while (qr.size()>0)
          {
            r = qr.get(0); qr.remove(0);
            c = qc.get(0); qc.remove(0);
            if (!visited[r][c])
            {
              visited[r][c] = true;
              if (grid[r][c]==Barn) continue; // Don't traverse through a barn
              for (int d=0;d<4;d++)
              {
                int nr = r+dr[d];
                int nc = c+dc[d];
                if (nr>=0 && nr<N && nc>=0 && nc<N && !visited[nr][nc])
                {
                  if (grid[r][c]!=Tree)
                  {
                    qr.add(nr);
                    qc.add(nc);
                  } 
                }
              }
            }
          }
          break;  
        }
      }
      // Check if all are connected
      for (int r=0;r<N;r++)
        for (int c=0;c<N;c++)
          if (!visited[r][c])
          {
            if (grid[r][c]==Farmer) return false;
            if (grid[r][c]>=Sheep && grid[r][c]<=SheepFull) return false;
            if (grid[r][c]==Barn) return false;
          }

      return true;
    }

    protected void generate()
    {
      N = randomInt(minN, maxN);
      B = randomInt(minB, maxB);
      C = randomInt(minC, maxC);

      sheepP = randomDouble(minPS, maxPS);
      farmerR = randomDouble(minRF, maxRF);
      treeP = randomDouble(minT, maxT);

      //Special cases
      if (seed == 1)
      {
        N = minN;
        C = minC;
        B = minB;
        sheepP = minPS*5; // Making sure we have more than 1 sheep for seed 1
        farmerR = minRF;
        treeP = minT;
      }
      else if (seed == 2)
      {
        N = maxN;
        C = maxC;
        B = maxB;
        sheepP = maxPS;
        farmerR = maxRF;
        treeP = maxT;
      }    
      
      //User defined parameters   
      if (parameters.isDefined("N"))
      {
        N = randomInt(parameters.getIntRange("N"), minN, maxN);
      }
      if (parameters.isDefined("C"))
      {
        C = randomInt(parameters.getIntRange("C"), minC, maxC);
      }   
      if (parameters.isDefined("B"))
      {
        B = randomInt(parameters.getIntRange("B"), minB, maxB);
      } 
      if (parameters.isDefined("sheepP"))
      {
        sheepP = randomDouble(parameters.getDoubleRange("sheepP"), minPS, maxPS);
      }
      if (parameters.isDefined("farmerR"))
      {
        farmerR = randomDouble(parameters.getDoubleRange("farmerR"), minRF, maxRF);
      }
      if (parameters.isDefined("treeP"))
      {
        treeP = randomDouble(parameters.getDoubleRange("treeP"), minT, maxT);
      }
      FarmerFull = (char)(Farmer+C);

      do
      {
        // place the sheep and trees
        while (true)
        {
          grid = new char[N][N];
          sheepIndex = new int[N][N];
          numSheep = 0;
          woolOnSheep = 0;
          for (int i=0; i<N; i++)
            for (int k=0; k<N; k++)
            {
              double a = randomDouble(0,1);
              if (a<sheepP)
              {
                // Start empty or full
                grid[i][k] = randomInt(0,1)==0 ? Sheep : SheepFull;
                woolOnSheep += grid[i][k]-Sheep;
                sheepIndex[i][k] = numSheep;
                numSheep++;
              }
              else if (a<sheepP+treeP)
              {
                grid[i][k]=Tree;
              }   
              else
              {
                grid[i][k]=Empty;
              }         
            }
          if (numSheep>0) break;      
        }

        // Fill in the growth rate for each sheep
        growthP = new double[numSheep];
        for (int i=0;i<numSheep;i++)
          growthP[i] = randomDouble(minPW, maxPW);
        // place the barns
        for (int i=0;i<B;i++)
        {
          int r,c;
          do
          {
            r = randomInt(0, N-1);
            c = randomInt(0, N-1);
          } while (grid[r][c]!=Empty);
          grid[r][c] = Barn;
        }
        // place the farmers
        numFarmers = Math.max(1, (int)(numSheep*farmerR));
        woolWithFarmers = 0;
        for (int i=0;i<numFarmers;i++)
        {
          int r,c;
          do
          {
            r = randomInt(0, N-1);
            c = randomInt(0, N-1);
          } while (grid[r][c]!=Empty);
          grid[r][c] = Farmer;
        }
        // make sure the farmers are connected with the sheep
      } while (!isFarmersSheepConnected());

      // Pre-compute movement and growth tables.
      woolGrow = new int[numSheep][1001];
      sheepMoves = new int[numSheep][1001];
      for (int si=0;si<numSheep;si++)
      {
        for (int i=0;i<=1000;i++)
        {
          woolGrow[si][i] = (randomDouble(0,1)<growthP[si]) ? 1 : 0;
          sheepMoves[si][i] = randomInt(0, 4);
        }
      }

      if (debug) 
      {
        System.out.println("Grid size, N = " + N);
        System.out.println("Carry capacity, C = " + C);
        System.out.println("Number of barns, B = " + B);
        System.out.println("Number of sheep, numSheep = " + numSheep);
        System.out.println("Number of farmers, numFarmers = " + numFarmers);
        System.out.println("Sheep probability, sheepP = " + sheepP);
        System.out.println("Farmer probability, farmerR = " + farmerR);
        System.out.println("Tree probability, treeP = " + treeP);
        System.out.println("Wool growth rate = ");
        for (int i=0; i<numSheep; i++)        
          System.out.print(growthP[i]+" ");
        System.out.println();
        System.out.println("Grid:");          
        for (int r=0; r<N; r++)
        {
          for (int c=0; c<N; c++)        
            System.out.print(grid[r][c]+" ");
          System.out.println();
        }         
      }
    }
    
    protected boolean isMaximize() {
        return true;
    }
    
    protected double run() throws Exception
    {     
      init();
    
      return runAuto();
    }
    
    
    protected double runAuto() throws Exception
    {   
      double score = callSolution();
      if (score < 0) {
        if (!isReadActive()) return getErrorScore();
        return fatalError();
      }
      return score;    
    }    

    protected void updateState()
    {
      if (hasVis())
      {      
        synchronized (updateLock) {
          addInfo("Time",  getRunTime());   
          addInfo("Turns", numTurns);
          addInfo("Wool on sheep", woolOnSheep);
          addInfo("Wool with farmers", woolWithFarmers);   
          addInfo("Score", score);         
        }
        updateDelay();
      }
    } 
    
    protected void timeout() {
      addInfo("Time", getRunTime());
      update();
    }    
      
    private double callSolution() throws Exception
    {
      int[] sheep = new int[numSheep];    
      writeLine(N);
      writeLine(C);
     
      //print grid
      for (int r=0; r<N; r++)
        for (int c=0; c<N; c++)        
          writeLine(""+grid[r][c]);

      flush();
      if (!isReadActive()) return -1;
      
      if (hasVis() && hasDelay()) {   
          synchronized (updateLock) {        
          }
          updateDelay();
      }         
      for (numTurns=1; numTurns <= MaxTurns; numTurns++)
      {            
        startTime();
        String line = readLine();
        stopTime();

        boolean[] sheared = new boolean[numSheep];
        String[] temp = line.trim().split(" ");
        if (temp.length%3!=0)
        {
          return fatalError("Cannot parse your output. Should be a multiple of 3 in the format R C M R C M ...");
        } else
        {
          // Move the farmers
          boolean[][] moved = new boolean[N][N];
          for (int i=0; i<temp.length; i+=3)
          {
            try
            {
              int row = Integer.parseInt(temp[i]);
              int col = Integer.parseInt(temp[i+1]);
              if (col<0 || col>=N || row<0 || row>=N) return fatalError("Coordinates outside the grid.");
              if (!(grid[row][col]>=Farmer && grid[row][col]<=FarmerFull))
              {
                return fatalError("Trying to move a non farmer cell. Row="+row+" Column="+col+" Cell="+grid[row][col]);
              }
              if (temp[i+2].length()!=1)
              {
                return fatalError("Invalid farmer move, should be a single character U,D,L,R or N.");
              }
              if (moved[row][col])
              {
                return fatalError("Farmer already moved, can't move a farmer multiple times in the same turn.");
              }
              char dir = temp[i+2].charAt(0);
              int nrow = row;
              int ncol = col;
              if (dir=='U') nrow = row-1;
              else if (dir=='D') nrow = row+1;
              else if (dir=='L') ncol = col-1;
              else if (dir=='R') ncol = col+1;
              else if (dir!='N')
              {
                return fatalError("Invalid farmer move, should be U,D,L,R or N.");
              }
              if (ncol<0 || ncol>=N || nrow<0 || nrow>=N)
              {
                // Out of bounds
                return fatalError("Farmer moved out of bounds.");
              } else
              {
                if (dir=='N') 
                {
                  // keep the farmer stationary
                  moved[row][col] = true;
                }
                else if (grid[nrow][ncol]==Tree)
                {
                  // Illegal to move into a tree
                  return fatalError("Farmer moved into a tree at row="+row+" column="+col);
                }
                else if (grid[nrow][ncol]==Empty)
                {
                  // basic move
                  grid[nrow][ncol] = grid[row][col];
                  grid[row][col] = Empty;
                  moved[nrow][ncol] = true;
                }
                else if (grid[nrow][ncol]==Barn)
                {
                  // Offload any wool to the barn
                  score += grid[row][col] - Farmer;
                  // Empty this farmer
                  grid[row][col] = Farmer;
                  moved[row][col] = true;
                }
                else if (grid[nrow][ncol]>=Farmer && grid[nrow][ncol]<=FarmerFull)        
                {
                  // Offload to another farmer, up to C
                  int passOn = Math.min(grid[row][col]-Farmer, FarmerFull-grid[nrow][ncol]);
                  grid[row][col] -= passOn;
                  grid[nrow][ncol] += passOn;
                  moved[row][col] = true;
                }
                else if (grid[nrow][ncol]>=Sheep && grid[nrow][ncol]<=SheepFull)        
                {
                  // Shear the sheep one layer at a time
                  int shearOff = Math.min(1, Math.min(FarmerFull-grid[row][col], grid[nrow][ncol]-Sheep));
                  grid[row][col] += shearOff;
                  grid[nrow][ncol] -= shearOff;
                  moved[row][col] = true;
                  if (shearOff>0)
                    sheared[sheepIndex[nrow][ncol]] = true; // sheared sheep don't move
                }  
              }
            }
            catch (Exception e)
            {
              if (debug) System.out.println(e.toString());
              return fatalError("Cannot parse your output");      
            }
          }
        }
        
        // Move the sheep
        int numS = 0;
        for (int r=0; r<N; r++)
        {
          for (int c=0; c<N; c++)
            if (grid[r][c]>=Sheep && grid[r][c]<=SheepFull)
            {
              sheep[numS++] = r+c*N;              
            }
        }
        for (int i=0;i<numS;i++)
        {
          int r = sheep[i]%N;
          int c = sheep[i]/N;
          int idx = sheepIndex[r][c];
          if (sheared[idx])
          {
            // Sheared sheep don't move or grow wool while being sheared
            continue;
          }
          // Grow some wool
          if (grid[r][c]>=Sheep && grid[r][c]<SheepFull)
          {
            grid[r][c] += woolGrow[idx][numTurns];
          }
          // Move the sheep
          int dir = sheepMoves[idx][numTurns];
          int nr = r+dr[dir];
          int nc = c+dc[dir];
          if (nc>=0 && nc<N && nr>=0 && nr<N && grid[nr][nc]==Empty)
          {
            grid[nr][nc] = grid[r][c];
            grid[r][c] = Empty;
            sheepIndex[nr][nc] = idx;
          }
        }
     
        // update sheep wool stats
        woolOnSheep = 0;
        for (int r=0; r<N; r++)
        {
          for (int c=0; c<N; c++)
            if (grid[r][c]>=Sheep && grid[r][c]<=SheepFull)
              woolOnSheep += grid[r][c]-Sheep;
        }
        // update farmer wool stats
        woolWithFarmers = 0;
        for (int r=0; r<N; r++)
        {
          for (int c=0; c<N; c++)
            if (grid[r][c]>=Farmer && grid[r][c]<=FarmerFull)
              woolWithFarmers += grid[r][c]-Farmer;
        }

        updateState();    //state 

        //output elapsed time and the grid
        if (numTurns<MaxTurns)
        {
          writeLine(""+getRunTime());
          //print grid
          for (int r=0; r<N; r++)
            for (int c=0; c<N; c++)        
              writeLine(""+grid[r][c]);
          flush();  
        }
      }
           
      return score;
    }

     
    protected void paintContent(Graphics2D g)
    { 
      adjustFont(g, Font.SANS_SERIF, Font.PLAIN, String.valueOf("1"), new Rectangle2D.Double(0, 0, 0.5, 0.5));        
      g.setStroke(new BasicStroke(0.005f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

      //draw grid      
      for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
        {
          g.setColor(Color.white);
          g.fillRect(c, r, 1, 1);
          g.setColor(Color.gray);       
          g.drawRect(c, r, 1, 1);      
        }  
      //draw objects
      for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
        {            
          if (parameters.isDefined("noImages"))
          {
            if (grid[r][c]!=Empty)
            {
              if (grid[r][c]>=Sheep && grid[r][c]<=SheepFull)
              {
                int wool = (grid[r][c]-Sheep)*120/3;
                g.setColor(new Color(100+wool, 100+wool, 100+wool));
                Ellipse2D t = new Ellipse2D.Double(c + 0.15, r + 0.15, 0.7, 0.7);
                g.fill(t);
                g.setColor(Color.black);
                drawString(g, Integer.toString(grid[r][c]-Sheep), new Rectangle2D.Double(c+0.5, r+0.5, 0, 0));
              }
              else if (grid[r][c]==Barn)
              {
                g.setColor(Color.red);
                g.fillRect(c, r, 1, 1);
              }
              else if (grid[r][c]==Tree)
              {
                g.setColor(Color.green);
                g.fillRect(c, r, 1, 1);
              }              
              else if (grid[r][c]>=Farmer && grid[r][c]<=FarmerFull)
              {
                int wool = (grid[r][c]-Farmer)*120/C;
                g.setColor(new Color(128+wool, 128+wool, 30));
                g.fillRect(c, r, 1, 1);
                g.setColor(Color.black);
                drawString(g, Integer.toString(grid[r][c]-Farmer), new Rectangle2D.Double(c+0.5, r+0.5, 0, 0));
              }
            }
          }
          else
          {
            if (grid[r][c]==Barn) g.drawImage(barnPic,c,r,1,1,null);
            else if (grid[r][c]==Tree) g.drawImage(treePic,c,r,1,1,null);
            else if (grid[r][c]>=Farmer && grid[r][c]<=FarmerFull) 
            {
              g.drawImage(farmerPic[grid[r][c]-Farmer],c,r,1,1,null);
              //g.setColor(Color.black);
              //drawString(g, Integer.toString(grid[r][c]-Farmer), new Rectangle2D.Double(c+0.5, r+0.5, 0, 0));
            }
            else if (grid[r][c]>=Sheep && grid[r][c]<=SheepFull)
            {
              g.drawImage(sheepPic[grid[r][c]-Sheep],c,r,1,1,null);    
              //g.setColor(Color.black);
              //drawString(g, Integer.toString(grid[r][c]-Sheep), new Rectangle2D.Double(c+0.5, r+0.5, 0, 0));      
            }
          }
        }
    }


    private void init()
    {
      numTurns = 0;
      score = 0;
      if (hasVis())
      {
        setDefaultDelay(100);    //this needs to be first

        if (!parameters.isDefined("noImages"))
        {
          // Load images
          farmerPic = new Image[C+1];
          sheepPic = new Image[4];
          farmerPic[0] = loadImage("images/farmer.png");
          Image woolPic = loadImage("images/wool.png");
          for (int i=1;i<=C;i++)
          {
            BufferedImage copyOfImage = new BufferedImage(128, 128, BufferedImage.TYPE_INT_ARGB);
            Graphics g = copyOfImage.createGraphics();
            g.drawImage(farmerPic[i-1], 0, 0, null);
            farmerPic[i] = copyOfImage;
            g.drawImage(woolPic, woolx[i-1], wooly[i-1], null);
          }
          for (int i=0;i<4;i++)
            sheepPic[i] = loadImage("images/sheep_"+i+".png");
          barnPic = loadImage("images/barn.png");
          treePic = loadImage("images/tree.png");
        }        
        
        setContentRect(0, 0, N, N);
        setInfoMaxDimension(21, 13);

        addInfo("Seed", seed);
        addInfo("N", N);
        addInfo("Carry Capacity", C);
        addInfo("Barns", B);
        addInfo("Farmers", numFarmers);
        addInfo("Sheep", numSheep);

        addInfoBreak();
        addInfo("Turns", numTurns);
        addInfo("Wool on sheep", woolOnSheep);
        addInfo("Wool with farmers", woolWithFarmers);   
        addInfo("Score", score);
        
        addInfoBreak();
        addInfo("Time", "-");
        update();
      }
    }

    Image loadImage(String name) {
      try{
        Image im=ImageIO.read(getClass().getResourceAsStream(name));
        return im;
      } catch (Exception e) { 
        return null;  
      }             
    }     

    public static void main(String[] args) {
        new MarathonController().run(args);
    }
}