import mdl
from display import *
from matrix import *
from draw import *

def run(filename):
    """
    This function runs an mdl script
    """
    p = mdl.parseFile(filename)

    if p:
        (commands, symbols) = p
    else:
        print("Parsing failed.")
        return

    view = [0,
            0,
            1];
    ambient = [50,
               50,
               50]
    light = [[0.5,
              0.75,
              1],
             [255,
              255,
              255]]

    color = [0, 0, 0]
    tmp = new_matrix()
    ident( tmp )

    stack = [ [x[:] for x in tmp] ]
    screen = new_screen()
    clear_screen(screen);
    zbuffer = new_zbuffer()
    tmp = []
    step_3d = 100
    consts = ''
    coords = []
    coords1 = []
    polygons = []
    symbols['.white'] = ['constants',
                         {'red': [0.2, 0.5, 0.5],
                          'green': [0.2, 0.5, 0.5],
                          'blue': [0.2, 0.5, 0.5]}]
    reflect = '.white'

    #print(symbols)
    for command in commands:
        #print(command)
        op = command['op']
        if (op == 'push'):
            stack.append( [x[:] for x in stack[-1]] )
        elif (op == 'pop'):
            stack.pop()
        elif (op == 'move'):
            tmp = make_translate(command['args'][0], command['args'][1], command['args'][2])
            matrix_mult( stack[-1], tmp )
            stack[-1] = [ x[:] for x in tmp]
        elif (op == 'rotate'):
            theta = command['args'][1] * (math.pi / 180)
            if command['args'][0] == 'x':
                tmp = make_rotX(theta)
            elif command['args'][0] == 'y':
                tmp = make_rotY(theta)
            else:
                tmp = make_rotZ(theta)
            matrix_mult( stack[-1], tmp )
            stack[-1] = [ x[:] for x in tmp]
        elif (op == 'scale'):
            tmp = make_scale(command['args'][0], command['args'][1], command['args'][2])
            matrix_mult( stack[-1], tmp )
            stack[-1] = [ x[:] for x in tmp]
        elif (op == 'box'):
            add_box(polygons,
                    command['args'][0], command['args'][1], command['args'][2],
                    command['args'][3], command['args'][4], command['args'][5])
            matrix_mult( stack[-1], polygons )
            if (command['constants'] != None):
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, command['constants'])
            else:
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, reflect)
            polygons = []
        elif (op == 'sphere'):
            add_sphere(polygons,
                       command['args'][0], command['args'][1], command['args'][2],
                       command['args'][3], step_3d)
            matrix_mult( stack[-1], polygons )
            if (command['constants'] != None):
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, command['constants'])
            else:
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, reflect)
            polygons = []
        elif (op == 'torus'):
            add_torus(polygons,
                      command['args'][0], command['args'][1], command['args'][2],
                      command['args'][3], command['args'][4], step_3d)
            matrix_mult( stack[-1], polygons )
            if (command['constants'] != None):
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, command['constants'])
            else:
                draw_polygons(polygons, screen, zbuffer, view, ambient, light, symbols, reflect)
            polygons = []
        elif (op == "constants"):
            # do nothing!
            a = 1;
        elif (op == "line"):
            add_edge( edges,
                      command['args'][0], command['args'][1], command['args'][2],
                      command['args'][3], command['args'][4], command['args'][5] )
            matrix_mult( stack[-1], edges )
            draw_lines(eges, screen, zbuffer, color)
            edges = []
        elif (op == "save"):
            save_extension(screen, command['args'][0] + ".png")
        elif (op == "display"):
            a = 1;
            display(screen)
