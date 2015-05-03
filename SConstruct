import os
import scons.utils
import scons.checks

SetOption('num_jobs', scons.utils.detectCPUs())

includedir = '{0}/include'.format(os.getcwd())

env = Environment(ENV = os.environ, CPPPATH=includedir)
config = env.Configure(custom_tests = {'CheckAllegro5': scons.checks.checkAllegro5(False)})
config.CheckAllegro5()
env = config.Finish()

#TODO Need to do separate checks later
env.ParseConfig('freetype-config --libs --cflags')

build_dir = 'build'
options = {'networking': False,
           'allegro5': True
          }

env.VariantDir(build_dir, 'src')
env.Library('r-tech1', env.SConscript('src/SConscript', variant_dir=build_dir, exports=['env', 'options']))
