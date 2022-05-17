/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack

  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.

  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {

  int i;
  struct matrix *tmp;
  struct stack *systems;
  screen s;
  zbuffer zb;
  color c;
  double step_3d = 100;
  double theta;

  //Lighting values here for easy access
  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  double light[2][3];
  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 255;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  double view[3];
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  //default reflective constants if none are set in script file
  struct constants white;
  white.r[AMBIENT_R] = 0.1;
  white.g[AMBIENT_R] = 0.1;
  white.b[AMBIENT_R] = 0.1;

  white.r[DIFFUSE_R] = 0.5;
  white.g[DIFFUSE_R] = 0.5;
  white.b[DIFFUSE_R] = 0.5;

  white.r[SPECULAR_R] = 0.5;
  white.g[SPECULAR_R] = 0.5;
  white.b[SPECULAR_R] = 0.5;

  //constants are a pointer in symtab, using one here for consistency
  struct constants *reflect;
  reflect = &white;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( s );
  clear_zbuffer(zb);
  c.red = 0;
  c.green = 0;
  c.blue = 0;

  struct matrix * polygons = new_matrix(4, 1000);

  //print_symtab();
  for (i=0;i<lastop;i++) {

    //printf("%d: ",i);
    switch(op[i].opcode)
      {
        case PUSH:
          push(systems);
          break;
        case POP:
          pop(systems);
          break;
        case MOVE:
          tmp = make_translate(op[i].op.move.d[0], op[i].op.move.d[1], op[i].op.move.d[2]);

          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          break;
        case ROTATE:
          if (op[i].op.rotate.axis == 0){
            tmp = make_rotX(op[i].op.rotate.degrees * M_PI/180);
          } else if (op[i].op.rotate.axis == 1){
            tmp = make_rotY(op[i].op.rotate.degrees * M_PI/180);
          } else if (op[i].op.rotate.axis == 2){
            tmp = make_rotZ(op[i].op.rotate.degrees * M_PI/180);
          }

          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          break;
        case SCALE:
          tmp = make_scale(op[i].op.scale.d[0], op[i].op.scale.d[1], op[i].op.scale.d[2]);

          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          break;
        case BOX: ;
          double x1 = op[i].op.box.d0[0];
          double y1 = op[i].op.box.d0[1];
          double z1 = op[i].op.box.d0[2];
          double x2 = op[i].op.box.d1[0];
          double y2 = op[i].op.box.d1[1];
          double z2 = op[i].op.box.d1[2];
          add_box(polygons, x1, y1, z1, x2, y2, z2);
          matrix_mult(peek(systems), polygons);
          if (op[i].op.box.constants != NULL) {
            if (op[i].op.box.constants->type == SYM_CONSTANTS){
              draw_polygons(polygons, s, zb, view, light, ambient, op[i].op.box.constants->s.c);
            }
          } else {
            draw_polygons(polygons, s, zb, view, light, ambient, reflect);
          }
          polygons->lastcol = 0;
          break;
        case SPHERE: ;
          x1 = op[i].op.sphere.d[0];
          y1 = op[i].op.sphere.d[1];
          z1 = op[i].op.sphere.d[2];
          double r = op[i].op.sphere.r;
          add_sphere(polygons, x1, y1, z1, r, step_3d);
          matrix_mult(peek(systems), polygons);
          if (op[i].op.sphere.constants != NULL){
            if (op[i].op.sphere.constants->type == SYM_CONSTANTS){
              //printf("constants: %f %f %f %f %f %f %f %f %f\n", op[i].op.sphere.constants->s.c->r[0], op[i].op.sphere.constants->s.c->r[1], op[i].op.sphere.constants->s.c->r[2], op[i].op.sphere.constants->s.c->g[0], op[i].op.sphere.constants->s.c->g[1], op[i].op.sphere.constants->s.c->g[2], op[i].op.sphere.constants->s.c->b[0], op[i].op.sphere.constants->s.c->b[1], op[i].op.sphere.constants->s.c->b[2]);
              draw_polygons(polygons, s, zb, view, light, ambient, op[i].op.sphere.constants->s.c);
            }
          } else {
            draw_polygons(polygons, s, zb, view, light, ambient, reflect);
          }
          polygons->lastcol = 0;
          break;
        case TORUS: ;
          x1 = op[i].op.torus.d[0];
          y1 = op[i].op.torus.d[1];
          z1 = op[i].op.torus.d[2];
          double r0 = op[i].op.torus.r0;
          double r1 = op[i].op.torus.r1;
          add_torus(polygons, x1, y1, z1, r0, r1, step_3d);
          matrix_mult(peek(systems), polygons);
          if (op[i].op.torus.constants != NULL){
            if (op[i].op.torus.constants->type == SYM_CONSTANTS){
              draw_polygons(polygons, s, zb, view, light, ambient, op[i].op.torus.constants->s.c);
            }
          } else {
            draw_polygons(polygons, s, zb, view, light, ambient, reflect);
          }
          polygons->lastcol = 0;
          break;
        case CONSTANTS: ;
          break;
        case LINE: ;
          x1 = op[i].op.line.p0[0];
          y1 = op[i].op.line.p0[1];
          z1 = op[i].op.line.p0[1];
          x2 = op[i].op.line.p1[0];
          y2 = op[i].op.line.p1[1];
          z2 = op[i].op.line.p1[1];
          if (op[i].op.line.constants != NULL){
            if (op[i].op.line.constants->type == SYM_CONSTANTS){
              draw_line(x1, y1, z1, x2, y2, z2, s, zb, c);
            }
          } else {
            draw_line(x1, y1, z1, x2, y2, z2, s, zb, c);
          }
          break;
        case SAVE: ;
          save_extension(s, op[i].op.save.p->name);
          break;
        case DISPLAY: ;
          display(s);
          break;
      }
  }
}
