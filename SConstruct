import os
import scons.utils
import scons.checks

SetOption('num_jobs', scons.utils.detectCPUs())

includedir = '{0}/include'.format(os.getcwd())

env = Environment(ENV = os.environ, CPPPATH=includedir, tools=['textfile', 'default'])
config = env.Configure(custom_tests = {'CheckAllegro5': scons.checks.checkAllegro5(False),
                                       'CheckFreetype': scons.checks.checkFreetype,
                                       'ConfigChecks': scons.checks.configChecks})
config.CheckAllegro5()
config.CheckFreetype()
config.ConfigChecks()
env = config.Finish()

if not env['HAVE_ALLEGRO5']:
    Exit(1)

build_dir = 'build'
options = {'networking': False,
           'allegro5': True
          }

env.VariantDir(build_dir, 'src')
libs = env.SConscript('src/SConscript', variant_dir=build_dir, exports=['env', 'options'])
rtech1 = env.StaticLibrary('lib/r-tech1', libs)
env.Default(rtech1)

# Install target and configuration
env.Install('{0}/lib'.format(env.installPrefix), rtech1)
env.Install('{0}/include'.format(env.installPrefix), 'include/r-tech1')

# Construct dependency cflags and libraries for pc script
def createList(content, modifier):
    deps = ''
    for item in content:
        deps += '-{0}{1} '.format(modifier, item) if 'r-tech1' not in item else ''
    return deps
pcflags = createList(env['CPPPATH'], 'I')
pclibs = createList(env['LIBS'], 'l')
pclibpaths = createList(env['LIBPATH'], 'L')

# PC script
replacelist = {
'%prefix%': env.installPrefix,
'%rtech1_version%': '1',
'%flags%': pcflags,
'%libs%': pclibs,
'%libpaths%': pclibpaths
}

pc_install = '{0}/lib/pkgconfig/r-tech1.pc'.format(env.installPrefix)

pc_copied = Command(build_dir + '/temp.pc.in', 'misc/r-tech1.pc.in', Copy('$TARGET', '$SOURCE'))
pc_script = env.Substfile(build_dir + '/temp.pc.in', SUBST_DICT = replacelist)
env.Depends(pc_script, pc_copied)
pc_mod = Command(build_dir + '/r-tech1.pc', build_dir + '/temp.pc', Copy('$TARGET', '$SOURCE'))
env.Depends(pc_mod, pc_script)
env.InstallAs(pc_install, pc_mod)

# Install
env.Alias('install', [env.installPrefix, pc_install])
env.Depends([env.installPrefix, pc_mod], rtech1)

# Uninstall target
env.Command("uninstall", None, Delete(FindInstalledFiles()))

