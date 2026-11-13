#ifndef ENGINE_SINGLETON_HPP
#define ENGINE_SINGLETON_HPP

namespace engine
{
    template<class T>
    class Singleton
    {
      public:
        Singleton( const Singleton& )            = delete;
        Singleton& operator=( const Singleton& ) = delete;
        Singleton( Singleton&& )                 = delete;
        Singleton& operator=( Singleton&& )      = delete;

        [[nodiscard]] static T& getInstance()
        {
            static T instance;
            return instance;
        }

      protected:
        Singleton()          = default;
        virtual ~Singleton() = default;
    };

} // namespace engine

#endif // ENGINE_SINGLETON_HPP
