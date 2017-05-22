__kernel void wrapcols(const int bheight, const int bwidth, __global char *inmat_d)
{
    int row = get_global_id(0);

    inmat_d[row*(bwidth + 2) + bwidth + 1] = inmat_d[row*(bwidth + 2) + 1]; 
	inmat_d[row*(bwidth + 2)] = inmat_d[row*(bwidth + 2) + bwidth]; 
}
 
__kernel void wraprows(const int bheight, const int bwidth, __global char *inmat_d)
{
    int col = get_global_id(0);
 
    if (col <= bwidth + 1) {
        inmat_d[(bwidth + 2)*(bheight + 1) + col] = inmat_d[(bwidth + 2) + col]; 
		inmat_d[col] = inmat_d[(bwidth + 2)*bheight + col]; 
    }
}
 
__kernel void GOL(const int bheight, const int bwidth, __global char *inmat_d, __global char *newmat_d)
{
	int row = get_global_id(0);
	int col = get_global_id(1);
 
	int count = 0;

	int ID = (bwidth + 2)*(row)+(col);
 
    // Get the number of neighbors for a given grid point
	count = inmat_d[ID + (bwidth + 2)]
			+ inmat_d[ID - (bwidth + 2)]
			+ inmat_d[ID + 1]
			+ inmat_d[ID - 1]
			+ inmat_d[ID + (bwidth + 3)]
			+ inmat_d[ID - (bwidth + 3)]
			+ inmat_d[ID - (bwidth + 1)]
			+ inmat_d[ID + (bwidth + 1)];

	//DIE
	if (count < 2 || count > 3) { newmat_d[ID] = 0; }
	//STAY
	else if (count == 2) { newmat_d[ID] = inmat_d[ID]; }
	//NEW
	else if (count == 3) { newmat_d[ID] = 1; }
}