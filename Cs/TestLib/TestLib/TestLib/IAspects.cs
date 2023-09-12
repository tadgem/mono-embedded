using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace IAspects
{
    // called at application startup
    public interface IOnInit
    {
        void Init();
    }

    // Called at begin play
    public interface IOnStart
    {
        void Start();
    }
    // every frame
    public interface IOnUpdate
    {   
        void Update(float delta);
    }

    // cleanup (program exit)
    public interface IOnCleanup
    {
        void Cleanup();
    }
}
