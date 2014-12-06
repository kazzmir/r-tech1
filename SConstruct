import os
import scons.utils
import scons.checks

SetOption('num_jobs', scons.utils.detectCPUs())

env = Environment(ENV = os.environ)
config = env.Configure(custom_tests = {'CheckAllegro5': scons.checks.checkAllegro5(False)})
config.CheckAllegro5()
env = config.Finish()
