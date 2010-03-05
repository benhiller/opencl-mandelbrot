__kernel void render(__global int *out) {
  int x0 = get_global_id(0);
  int y0 = get_global_id(1);
  int2 coordinates = (int2)(x0, y0);

  int x = 0;
  int y = 0;

  int iteration = 0;
  int max_iteration = 256;
  // This will probably be horrible, due the high amount of branching
  while(x^2 + y^2 <= 4 && iteration < max_iteration) {
    int xtemp = x^2 -y^2 + x0;
    y = 2*x*y + y0;
    x = xtemp;
    iteration++;
  }

  if(iteration == max_iteration) {
    out[x0][y0] = (char4)(0,0,0, 0);
  } else {
    int c = 256 - iteration;
    out[x0][y0] = (char4)(c,c,c, 0);
  }

}
