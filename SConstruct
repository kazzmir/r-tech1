import os
import scons_rtech1.utils
import scons_rtech1.checks
import scons_rtech1.env

SetOption('num_jobs', scons_rtech1.utils.detectCPUs())

# FIXME: use Dir('.').rel_path(Dir('#"))
includedir = '{0}/include'.format(os.getcwd())

env = Environment(ENV = os.environ, CPPPATH=includedir, tools=['textfile', 'default'])

if scons_rtech1.utils.useAndroid():
    env = scons.env.android(env)
if scons_rtech1.utils.useAndroidX64():
    env = scons.env.androidx64(env)

if not scons_rtech1.utils.isVerbose():
    env = scons_rtech1.utils.less_verbose(env)

root = '.'
SConscript('SConscript', exports = ['env', 'root'])
