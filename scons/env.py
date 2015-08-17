from SCons.Script import Split

def android(env):
    # Sets up the environment for Google Android
    def setup(pre, x):
        return '%s%s' % (pre, x)
    
    platform = 'android-9'
    arch = 'armeabi-v7a'
    path = '/opt/android/android-toolchain'
    # bin_path = setup(path, '/arm-linux-androideabi-4.4.3/bin')
    bin_path = setup(path, '/bin')
    prefix = 'arm-linux-androideabi-'
    def set_prefix(x):
        return '%s%s' % (prefix, x)
    env['CC'] = set_prefix('gcc')
    env['LD'] = set_prefix('ld')
    env['CXX'] = set_prefix('g++')
    env['AS'] = set_prefix('as')
    env['AR'] = set_prefix('ar')
    env['OBJCOPY'] = set_prefix('objcopy')

    base = setup(path, '/user/%(arch)s' % {'arch': arch})
    
    env.PrependENVPath('PKG_CONFIG_PATH', base + '/lib/pkgconfig')

    env.Append(CPPPATH = ['%s/include' % base,
                          # '%s/include/allegro5' % base
                          ])
    
    #env.Append(CPPPATH = [setup(path, '/arm-linux-androideabi-4.4.3/include'), 
    #                      setup(path, '/platforms/%s/arch-arm/usr/include' % platform),
    #                      setup(path, '/platforms/%s/arch-arm/usr/include/SDL' % platform),
    #                      setup(path, '/platforms/%s/arch-arm/usr/include/freetype' % platform),
    #                      setup(path, '/sources/cxx-stl/gnu-libstdc++/include')
    #                     ])
    env.Append(CPPDEFINES = Split("""ANDROID __ARM_ARCH_5__ __ARM_ARCH_5T__ __ARM_ARCH_5E__ __ARM_ARCH_5TE__"""))
    # flags = ['-fpic', '-fexceptions', '-ffunction-sections', '-funwind-tables', '-fstack-protector',  '-Wno-psabi', '-march=armv5te', '-mtune=xscale', '-msoft-float', '-mthumb', '-Os', '-fomit-frame-pointer', '-fno-strict-aliasing', '-finline-limit=64',]
    flags = ['-shared', '-fpic', '-fexceptions', '-ffunction-sections', '-funwind-tables', '-Wno-psabi', '-march=armv5te', '-mtune=xscale', '-msoft-float', '-mthumb', '-Os', '-fomit-frame-pointer', '-fno-strict-aliasing', '-finline-limit=64']
    # linkflags = flags + ['-Wl,--allow-shlib-undefined']
    linkflags = flags + ['-Wl,--no-undefined']
    # libs = ['freetype', 'png', 'SDL', 'm', 'log', 'jnigraphics', 'c', 'm', 'supc++',]
    # Copy the static stdc++ from gnu-libstdc++
    # gnustdlib = env.InstallAs('misc/libgnustdc++.a', '/opt/android/sources/cxx-stl/gnu-libstdc++/libs/armeabi/libstdc++.a')
    # libs = Split("""freetype2-static png SDL m log c jnigraphics supc++ EGL GLESv2 GLESv1_CM z gnustdc++""")
    libs = Split("""freetype2-static png m log c jnigraphics EGL GLESv2 GLESv1_CM z gnustl_static""")
    env.Append(CCFLAGS = flags)
    env.Append(CXXFLAGS = flags)
    env.Append(LINKFLAGS = linkflags)
    env.Append(CPPPATH = ['#src/android'])
    env['LINKCOM'] = '$CXX $LINKFLAGS -Wl,--start-group $SOURCES $ARCHIVES $_LIBDIRFLAGS $_LIBFLAGS -Wl,--end-group -o $TARGET'
    # Hack to put libstdc++ at the end
    # env['LINKCOM'] = '$CXX $LINKFLAGS $SOURCES $_LIBDIRFLAGS $_LIBFLAGS /opt/android/sources/cxx-stl/gnu-libstdc++/libs/armeabi/libstdc++.a -o $TARGET'
    # env['LINKCOM'] = '$CXX $LINKFLAGS $SOURCES $_LIBDIRFLAGS $_LIBFLAGS -o $TARGET'
    env.Append(LIBS = libs)
    env.Append(LIBPATH = ['%s/lib' % base,
         #setup(path, '/platforms/%s/arch-arm/usr/lib' % platform),
                          ])
    
    env.PrependENVPath('PATH', bin_path)
    return env
