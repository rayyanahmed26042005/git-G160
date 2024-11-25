import React, { useState } from 'react';
import './BillPage.css';

const BillPage = () => {
    const [totalUsage, setTotalUsage] = useState(null);
    const [bill, setBill] = useState(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState('');
    const [successMessage, setSuccessMessage] = useState('');

    const ratePerHour = 5; // Rate per hour in your currency

    const fetchUsageData = async () => {
        setLoading(true);
        setError('');
        setSuccessMessage('');
        setTotalUsage(null);
        setBill(null);

        try {
            const response = await fetch('http://localhost:8080/api/devices/calculateUsage');

            if (!response.ok) {
                const errorData = await response.json();
                setError(errorData.message || 'Failed to fetch usage data.');
                return;
            }

            const data = await response.json();

            const usage = parseFloat(data.totalDuration) || 0; // Adjust key based on backend
            const calculatedBill = usage * ratePerHour;

            setTotalUsage(usage.toFixed(2)); // Display with 2 decimal places
            setBill(calculatedBill.toFixed(2)); // Display with 2 decimal places
        } catch (err) {
            console.error('Fetch Error:', err.message);
            setError('An error occurred while fetching the usage data.');
        } finally {
            setLoading(false);
        }
    };

    const clearUsageData = async () => {
        setLoading(true);
        setError('');
        setSuccessMessage('');

        try {
            const response = await fetch('http://localhost:8080/api/devices/clearUsage', {
                method: 'DELETE',
            });

            if (!response.ok) {
                const errorData = await response.json();
                setError(errorData.message || 'Failed to clear usage data.');
                return;
            }

            setTotalUsage(null);
            setBill(null);
            setSuccessMessage('All usage data cleared successfully.');
        } catch (err) {
            console.error('Clear Error:', err.message);
            setError('An error occurred while clearing the usage data.');
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="bill-page">
            <h1>Electricity Usage & Bill Calculator</h1>
            <div className="form">
                <button onClick={fetchUsageData} disabled={loading}>
                    {loading ? 'Calculating...' : 'Calculate Total Bill'}
                </button>
            </div>

            {error && <p className="error">{error}</p>}
            {successMessage && <p className="success">{successMessage}</p>}

            {!loading && !error && totalUsage && bill && (
                <div className="result-container">
                    <p>
                        <strong>Total Usage:</strong> {totalUsage} hours
                    </p>
                    <p>
                        <strong>Total Electricity Bill:</strong> ₹{bill}
                    </p>
                    <button onClick={clearUsageData} disabled={loading} className="clear-btn">
                        {loading ? 'Clearing...' : 'Clear Data'}
                    </button>
                </div>
            )}
        </div>
    );
};

export default BillPage;
