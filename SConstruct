import os
import scons.utils
import scons.checks

SetOption('num_jobs', scons.utils.detectCPUs())

includedir = '{0}/include'.format(os.getcwd())

env = Environment(ENV = os.environ, CPPPATH=includedir, tools=['textfile', 'default'])
config = env.Configure(custom_tests = {'CheckAllegro5': scons.checks.checkAllegro5(scons.checks.debug()),
                                       'CheckFreetype': scons.checks.checkFreetype,
                                       'ConfigChecks': scons.checks.configChecks})
config.CheckAllegro5()
config.CheckFreetype()
config.ConfigChecks()
env = config.Finish()

env = scons.utils.less_verbose(env)

if not env['HAVE_ALLEGRO5']:
    Exit(1)

if scons.checks.debug():
    env.Append(CXXFLAGS = ['-g3','-ggdb', '-Werror'])

build_dir = 'build/release' if not scons.checks.debug() else 'build/debug'
options = {'networking': False,
           'allegro5': True
          }

libname = 'lib/r-tech1' if not scons.checks.debug() else 'lib/r-tech1-debug'

env.VariantDir(build_dir, 'src')
libs = env.SConscript('src/SConscript', variant_dir=build_dir, exports=['env', 'options'])
rtech1 = env.StaticLibrary(libname, libs)
Alias('rtech1', rtech1)

tests_build_dir = os.path.join(build_dir, 'tests')
unit_tests = SConscript('tests/SConscript', variant_dir = tests_build_dir, exports = ['env', 'rtech1'], duplicate=0)
env.Depends(unit_tests, rtech1)

if os.access(env.installPrefix, os.W_OK):
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
    '%lib%': 'r-tech1' if not scons.checks.debug() else 'r-tech1-debug',
    '%prefix%': env.installPrefix,
    '%rtech1_version%': '1',
    '%flags%': pcflags,
    '%libs%': pclibs,
    '%libpaths%': pclibpaths
    }
    
    def script(name):
        pc_install = '{0}/lib/pkgconfig/{1}.pc'.format(env.installPrefix, name)
        pc_copied = Command(build_dir + '/temp.pc.in', 'misc/r-tech1.pc.in'.format(name), Copy('$TARGET', '$SOURCE'))
        pc_script = env.Substfile(build_dir + '/temp.pc.in', SUBST_DICT = replacelist)
        env.Depends(pc_script, pc_copied)
        pc_mod = Command(build_dir + '/{0}.pc'.format(name), build_dir + '/temp.pc', Copy('$TARGET', '$SOURCE'))
        env.Depends(pc_mod, pc_script)
        env.InstallAs(pc_install, pc_mod)
        return pc_mod, pc_install
        
    pc_mod, pc_install = script('r-tech1') if not scons.checks.debug() else script('r-tech1-debug')

    # Install
    env.Alias('install', [env.installPrefix, pc_install])
    env.Depends([env.installPrefix, pc_mod], rtech1)

    # Uninstall target
    env.Command("uninstall", None, Delete(FindInstalledFiles()))
else:
    def needsudo(target, source, env):
        print 'No write privaleges to {0}, run target [{1}] as sudo'.format(env.installPrefix, target[0])
    env.Command('install', None, needsudo)
    env.Depends('install', ['rtech1', 'tests'])
    env.Command('uninstall', None, needsudo)
    env.Depends('uninstall', ['rtech1', 'tests'])

env.Default(rtech1)
env.Default(unit_tests)

for test in unit_tests:
    orig = str(test).translate(None,'[]\'')
    to = orig.replace('{0}/tests/'.format(build_dir), '')
    print orig, to
    copy = Command('bin/{0}'.format(to), orig, Copy('$TARGET', '$SOURCE'))
    env.Depends(copy, test)
    env.Default(copy)
