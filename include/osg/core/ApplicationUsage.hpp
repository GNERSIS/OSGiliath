/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Command-line help text registry. Collects and formats usage
 * descriptions for --help output.
 */
#pragma once

#include <map>
#include <osg/core/Referenced.hpp>
#include <ostream>
#include <string>

namespace osg
{

    class OSG_EXPORT ApplicationUsage : public osg::Referenced
    {
        public:

            static ApplicationUsage*
            instance();

            ApplicationUsage()
            {
            }

            ApplicationUsage( const std::string& commandLineUsage );

            typedef std::map<std::string, std::string> UsageMap;

            /** The ApplicationName is often displayed when logging errors, and
             * frequently incorporated into the Description (below). */
            void
            setApplicationName( const std::string& name )
            {
                _applicationName = name;
            }

            const std::string&
            getApplicationName() const
            {
                return _applicationName;
            }

            /** If non-empty, the Description is typically shown by the Help Handler
             * as text on the Help display (which also lists keyboard abbreviations. */
            void
            setDescription( const std::string& desc )
            {
                _description = desc;
            }

            const std::string&
            getDescription() const
            {
                return _description;
            }

            enum Type
            {
                NO_HELP                = 0X0,
                COMMAND_LINE_OPTION    = 0X1,
                ENVIRONMENTAL_VARIABLE = 0X2,
                KEYBOARD_MOUSE_BINDING = 0X4,
                HELP_ALL               = KEYBOARD_MOUSE_BINDING |
                    ENVIRONMENTAL_VARIABLE |
                    COMMAND_LINE_OPTION,
            };

            void
            addUsageExplanation( Type               type,
                                 const std::string& option,
                                 const std::string& explanation );

            void
            setCommandLineUsage( const std::string& explanation )
            {
                _commandLineUsage = explanation;
            }

            const std::string&
            getCommandLineUsage() const
            {
                return _commandLineUsage;
            }

            void
            addCommandLineOption( const std::string& option,
                                  const std::string& explanation,
                                  const std::string& defaultValue = "" );

            void
            setCommandLineOptions( const UsageMap& usageMap )
            {
                _commandLineOptions = usageMap;
            }

            const UsageMap&
            getCommandLineOptions() const
            {
                return _commandLineOptions;
            }

            void
            setCommandLineOptionsDefaults( const UsageMap& usageMap )
            {
                _commandLineOptionsDefaults = usageMap;
            }

            const UsageMap&
            getCommandLineOptionsDefaults() const
            {
                return _commandLineOptionsDefaults;
            }

            void
            addEnvironmentalVariable( const std::string& option,
                                      const std::string& explanation,
                                      const std::string& defaultValue = "" );

            void
            setEnvironmentalVariables( const UsageMap& usageMap )
            {
                _environmentalVariables = usageMap;
            }

            const UsageMap&
            getEnvironmentalVariables() const
            {
                return _environmentalVariables;
            }

            void
            setEnvironmentalVariablesDefaults( const UsageMap& usageMap )
            {
                _environmentalVariablesDefaults = usageMap;
            }

            const UsageMap&
            getEnvironmentalVariablesDefaults() const
            {
                return _environmentalVariablesDefaults;
            }

            void
            addKeyboardMouseBinding( const std::string& prefix,
                                     int                key,
                                     const std::string& explanation );
            void
            addKeyboardMouseBinding( int                key,
                                     const std::string& explanation );
            void
            addKeyboardMouseBinding( const std::string& option,
                                     const std::string& explanation );

            void
            setKeyboardMouseBindings( const UsageMap& usageMap )
            {
                _keyboardMouse = usageMap;
            }

            const UsageMap&
            getKeyboardMouseBindings() const
            {
                return _keyboardMouse;
            }

            void
            getFormattedString( std::string&    str,
                                const UsageMap& um,
                                unsigned int    widthOfOutput = 80,
                                bool            showDefaults  = false,
                                const UsageMap& ud            = UsageMap() );

            void
            write( std::ostream&   output,
                   const UsageMap& um,
                   unsigned int    widthOfOutput = 80,
                   bool            showDefaults  = false,
                   const UsageMap& ud            = UsageMap() );

            void
            write( std::ostream& output,
                   unsigned int  type          = COMMAND_LINE_OPTION,
                   unsigned int  widthOfOutput = 80,
                   bool          showDefaults  = false );

            void
            writeEnvironmentSettings( std::ostream& output );

        protected:

            virtual ~ApplicationUsage()
            {
            }

            std::string _applicationName;
            std::string _description;
            std::string _commandLineUsage;
            UsageMap    _commandLineOptions;
            UsageMap    _environmentalVariables;
            UsageMap    _keyboardMouse;
            UsageMap    _environmentalVariablesDefaults;
            UsageMap    _commandLineOptionsDefaults;
    };

    class ApplicationUsageProxy
    {
        public:

            /** register an explanation of commandline/environmentvariable/keyboard mouse
             * usage.*/
            ApplicationUsageProxy( ApplicationUsage::Type type,
                                   const std::string&     option,
                                   const std::string&     explanation )
            {
                ApplicationUsage::instance()->addUsageExplanation( type,
                                                                   option,
                                                                   explanation );
            }
    };

}
