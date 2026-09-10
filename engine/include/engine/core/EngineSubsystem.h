#pragma once
#include <map>
#include <memory>
#include <print>

namespace eng
{
	enum class SubsystemId
	{
		LOGGER = 0,
		FILESYSTEM,
		WINDOW,
		RENDERER,
		GUI
	};


	class EngineSubsystem {
        public:
            virtual ~EngineSubsystem() = default;
            virtual void init() {}
            virtual void shutdown() {}

	};

	class SubsystemManager
	{
        public:

			SubsystemManager() = default;

			template<SubsystemId id, class T>
			SubsystemManager& add()
			{ 
				systems.emplace(id, std::move(std::make_unique<T>()));
				return *this;
			}


			template<class T>
			T* GetComponent(SubsystemId id)
			{
                return dynamic_cast<T*>(systems.find(id)->second.get());
			}

			void start()
			{ 
				for (const auto& elem : systems)
				{
                    elem.second->init();
				}
			}

			~SubsystemManager()
			{ 
				for (auto it = systems.crbegin(); it != systems.crend(); it++)
				{
                    it->second->shutdown();
				}
			}


        private:
           std::map<SubsystemId, std::unique_ptr<EngineSubsystem>> systems;

	};
}