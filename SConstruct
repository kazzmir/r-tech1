import os
import scons.utils
import scons.checks
import scons.env

SetOption('num_jobs', scons.utils.detectCPUs())

includedir = '{0}/include'.format(os.getcwd())

env = Environment(ENV = os.environ, CPPPATH=includedir, tools=['textfile', 'default'])

if scons.utils.useAndroid():
    env = scons.env.android(env)
if scons.utils.useAndroidX64():
    env = scons.env.androidx64(env)

if not scons.utils.isVerbose():
    env = scons.utils.less_verbose(env)

SConscript('SConscript', exports = ['env'])
