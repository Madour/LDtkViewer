// Created by Modar Nasser on 28/02/2022.


template <typename T>
void Shader::setUniform(const std::string& uniform, const T& val) {
    auto location = getUniformLocation(uniform);
    if constexpr(std::is_same_v<T, float>) {
        glUniform1f(location, val);
    } else if constexpr(std::is_same_v<T, int>) {
        glUniform1i(location, val);
    } else if constexpr(std::is_same_v<T, unsigned>) {
        glUniform1ui(location, val);
    }
}

template <typename T, glm::length_t S>
void Shader::setUniform(const std::string& uniform, const glm::vec<S, T, glm::defaultp>& vec) {
    auto location = getUniformLocation(uniform);
    if constexpr(std::is_same_v<T, float>) {
        if constexpr(S == 1)
            glUniform1f(location, vec.x);
        else if constexpr(S == 2)
            glUniform2f(location, vec.x, vec.y);
        else if constexpr(S == 3)
            glUniform3f(location, vec.x, vec.y, vec.z);
        else if constexpr(S == 4)
            glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
    } else if constexpr(std::is_same_v<T, int>) {
        if constexpr(S == 1)
            glUniform1i(location, vec.x);
        else if constexpr(S == 2)
            glUniform2i(location, vec.x, vec.y);
        else if constexpr(S == 3)
            glUniform3i(location, vec.x, vec.y, vec.z);
        else if constexpr(S == 4)
            glUniform4i(location, vec.x, vec.y, vec.z, vec.w);
    } else if constexpr(std::is_same_v<T, unsigned>) {
        if constexpr(S == 1)
            glUniform1ui(location, vec.x);
        else if constexpr(S == 2)
            glUniform2ui(location, vec.x, vec.y);
        else if constexpr(S == 3)
            glUniform3ui(location, vec.x, vec.y, vec.z);
        else if constexpr(S == 4)
            glUniform4ui(location, vec.x, vec.y, vec.z, vec.w);
    }
}

template <typename T>
void Shader::setUniform(const std::string& uniform, const std::initializer_list<T>& arr) {
    auto location = getUniformLocation(uniform);
    if constexpr(std::is_same_v<T, float>) {
        glUniform1fv(location, std::size(arr), std::data(arr));
    } else if constexpr(std::is_same_v<T, int>) {
        glUniform1iv(location, std::size(arr), std::data(arr));
    } else if constexpr(std::is_same_v<T, unsigned>) {
        glUniform1uiv(location, std::size(arr), std::data(arr));
    }
}

template <typename T, glm::length_t S>
void Shader::setUniform(const std::string& uniform, const std::initializer_list<glm::vec<S, T, glm::defaultp>>& arr) {
    auto location = getUniformLocation(uniform);
    if constexpr(std::is_same_v<T, float>) {
        if constexpr(S == 1)
            glUniform1fv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 2)
            glUniform2fv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 3)
            glUniform3fv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 4)
            glUniform4fv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
    } else if constexpr(std::is_same_v<T, int>) {
        if constexpr(S == 1)
            glUniform1iv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 2)
            glUniform2iv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 3)
            glUniform3iv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 4)
            glUniform4iv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
    } else if constexpr(std::is_same_v<T, unsigned>) {
        if constexpr(S == 1)
            glUniform1uiv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 2)
            glUniform2uiv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 3)
            glUniform3uiv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
        else if constexpr(S == 4)
            glUniform4uiv(location, std::size(arr), reinterpret_cast<const T*>(std::data(arr)));
    }
}
