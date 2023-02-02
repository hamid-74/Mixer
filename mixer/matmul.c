
 
// Driver code
int main(int argc, char *argv[])
{
    // R1 = 4, C1 = 4 and R2 = 4, C2 = 4 (Update these
    // values in MACROs)
    float mat1[6][6];
 
    float mat2[6][6];
    
    float rslt[6][6];
 
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            rslt[i][j] = 0;
 
            for (int k = 0; k < 6; k++) {
                rslt[i][j] += mat1[i][k] * mat2[k][j];
            }
 
    
        }
 
        
    }


 
    return 0;
}