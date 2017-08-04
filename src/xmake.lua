
target 'xval'
    set_kind 'shared'

    if os.host() ~= 'windows' then
        add_cxxflags '-std=c++11'
    end
    add_defines 'DLL_EXPORT'
    add_headers 'xval*.h'
    add_files 'xval_*.cpp'
