int index(int x, int y, int width) {
  return width*y + x*3;
}

__kernel void render(__global char *out) {
  int x_dim = get_global_id(0);
  int y_dim = get_global_id(1);
  size_t global_size_x = get_global_size(0);
  size_t global_size_y = get_global_size(1);
  int idx = index(x_dim, y_dim, global_size_x);

  float x_origin = (float) x_dim / global_size_x;
  float y_origin = (float) y_dim / global_size_y;

  float x = 0.0;
  float y = 0.0;

  int iteration = 0;
  int max_iteration = 256;
  // This will probably be horrible, due the high amount of branching
  while(x*x + y*y <= 4 && iteration < max_iteration) {
    float xtemp = x*x - y*y + x_origin;
    y = 2*x*y + y_origin;
    x = xtemp;
    iteration++;
  }

  if(iteration == max_iteration) {
    out[idx] = 0;
    out[idx + 1] = 0;
    out[idx + 2] = 0;
  } else {
    out[idx] = iteration;
    out[idx + 1] = iteration;
    out[idx + 2] = iteration;
  }

}
