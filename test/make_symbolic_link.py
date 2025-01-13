

 # Symbolic link(current res dir to ../debug/res)
 
import os
import sys
 
def make_symbolic_link():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    res_dir = os.path.join(current_dir, '..\\build\\source\\phonon\\res')
    if not os.path.exists(res_dir):
        os.symlink(os.path.join(current_dir, 'res'), res_dir)
    print('Symbolic link created: %s -> %s' % (res_dir, os.path.join(current_dir, 'res')))
    return res_dir

if __name__ == '__main__':
    make_symbolic_link()