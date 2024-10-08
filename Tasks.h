#pragma once

/// @brief A simple, efficient task scheduler for embedded microcontrollers
///
/// This task scheduler is designed to be simple to use and to have very little
/// overhead in time and memory. Some compile-time tricks are used to create a
/// static list of tasks, while keeping the application-level code simple with
/// a minimum of boilerplate.
/// 
/// Usage
/// -----
/// To define a set of tasks:
/// 
/// 1. Define each task as a subclass of Tasks::Task like this.
/// A single instance of this class will be created automatically.
/// @note This declaration uses the "curiously recurring template pattern".
/// @see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
/// @code
/// class ExampleTask : public Tasks::Task<ExampleTask>
/// {
/// public:
///     // Task execution interval in microseconds
///     unsigned intervalMicros() const { return 1'000'000; }
/// 
///     // Task initialization, called once at program start
///     void init()
///     {
///         // task initialization code
///     }
/// 
///     // Main task function, executed at (approximately) the specified interval
///     void execute()
///     {
///         // task main code
///     }
/// };
/// @endcode
/// 2. Make a list of all the tasks by declaring a specialization of Tasks::TaskList.
/// Any tasks not currently required (e.g. for debugging) can be commented out
/// and the unused task code will not be compiled into the executable.
/// @code
/// using TaskList = Tasks::TaskList<
///     ExampleTask,
///     AnotherTask,
///     AndAnotherTask
/// >;
/// @endcode
/// 3. In main(), initialize all the tasks and then execute them repeatedly.
/// @code
/// int main()
/// {
///     // Initialize all the tasks.
///     TaskList::initAll();
///
///     // Execute all the tasks repeatedly, at their specified time intervals.
///     while (true) {
///         TaskList::runAll();
///     }
///
///     return 0;
/// }
/// @endcode

namespace Tasks {

/// @brief  Base class for application-defined tasks
template<typename SUB>
class Task
{
public:
    /// @brief If it's time to call execute(), do so
    /// @param now Current time
    void tick(absolute_time_t now)
    {
        if (timeIsReached(now, timer)) {
            timer = make_timeout_time_us(subclass()->intervalMicros());
            subclass()->execute();
        }
    }

private:
    SUB* subclass() { return static_cast<SUB*>(this); }

    /// @brief Keeps track of the next time this task should be executed
    absolute_time_t timer = from_us_since_boot_constexpr(0);
};

/// @brief A static list of Task that is initialized at compile time
/// @tparam ...TASKS List of Task subclasses
template<typename... TASKS>
class TaskList
{
public:
    /// @brief Initialize all the tasks
    static void initAll()
    {
        ((taskInstance<TASKS>.init()), ...);
    }

    /// @brief Execute all the tasks repeatedly, at their specified time intervals
    static void runAll()
    {
        absolute_time_t now = get_absolute_time();
        ((taskInstance<TASKS>.tick(now)), ...);
    }

private:
    /// @brief There is one static instance of each subclass of Task
    /// @tparam TASK_T A subclass of Task
    template<typename TASK_T>
    static inline TASK_T taskInstance;
};

} // namespace Tasks
